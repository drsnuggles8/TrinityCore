#pragma once

// Small frozen inference only. No engine access, action execution or training runtime.
#include <rapidjson/document.h>
#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <deque>
#include <span>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

namespace Playerbot::V2::Fleet::TrainingTarget
{
inline constexpr std::array<std::string_view, 13> FeatureNames = {
    "self_health_fraction", "self_mana_fraction", "self_level_scaled", "self_in_combat", "bag_free_scaled",
    "enemy_health_fraction", "enemy_health_vs_self", "enemy_level_difference", "distance_scaled", "height_difference",
    "line_of_sight", "tagged_by_other", "quest_relevant"};
using Features = std::array<float, 13>;

struct CategoricalChoice { std::size_t index; std::vector<double> probabilities; };
inline CategoricalChoice Sample(std::span<float const> scores, double temperature, double unit)
{
    if (scores.empty() || scores.size() > 16 || !std::isfinite(temperature) || temperature < 0.05 || temperature > 5 ||
        !std::isfinite(unit) || unit < 0 || unit >= 1 ||
        !std::all_of(scores.begin(),scores.end(),[](float value) { return std::isfinite(value); }))
        throw std::runtime_error("Invalid categorical target menu");
    double top = *std::max_element(scores.begin(),scores.end());
    std::vector<double> probabilities;
    double total = 0;
    for (float score : scores) { double weight = std::exp((double(score)-top)/temperature); probabilities.push_back(weight); total += weight; }
    double cumulative = 0;
    std::size_t chosen = scores.size()-1;
    bool found = false;
    for (std::size_t i = 0; i < probabilities.size(); ++i)
    {
        probabilities[i] /= total;
        cumulative += probabilities[i];
        if (!found && unit < cumulative) { chosen = i; found = true; }
    }
    return {chosen,std::move(probabilities)};
}

struct OwnState
{
    double health, max_health, mana, max_mana, level, bag_free;
    bool in_combat;
};
struct EnemyState
{
    double health, max_health, level, dx, dy, dz;
    bool in_los, tagged_by_other, quest_relevant;
};
inline Features Encode(OwnState const& own, EnemyState const& enemy)
{
    auto clamp = [](double value, double low, double high) { return static_cast<float>(std::clamp(value, low, high)); };
    double distance = std::sqrt(enemy.dx*enemy.dx + enemy.dy*enemy.dy + enemy.dz*enemy.dz);
    return {clamp(own.health/std::max(1.0, own.max_health), 0, 1), clamp(own.mana/std::max(1.0, own.max_mana), 0, 1),
        clamp(own.level/6, 0, 2), float(own.in_combat), clamp(own.bag_free/20, 0, 4),
        clamp(enemy.health/std::max(1.0, enemy.max_health), 0, 1),
        clamp(enemy.max_health/std::max(1.0, own.max_health), 0, 3)/3,
        clamp((enemy.level-own.level)/6, -1, 1), clamp(distance/40, 0, 2), clamp(enemy.dz/20, -1, 1),
        float(enemy.in_los), float(enemy.tagged_by_other), float(enemy.quest_relevant)};
}

class Model
{
    struct Layer { std::size_t inputs = 0, outputs = 0; std::vector<float> weights, bias; };
    std::array<Layer, 3> layers_;
    static rapidjson::Value const& Member(rapidjson::Value const& value, char const* key)
    {
        if (!value.IsObject() || !value.HasMember(key)) throw std::runtime_error("Missing target-model field");
        return value[key];
    }
    static float Number(rapidjson::Value const& value)
    {
        if (!value.IsNumber() || !std::isfinite(value.GetDouble()) || std::abs(value.GetDouble()) > 10000)
            throw std::runtime_error("Invalid target-model number");
        return static_cast<float>(value.GetDouble());
    }
public:
    std::string checkpoint_sha256;
    std::uint64_t epoch = 0, optimizer_step = 0;

    float Score(std::span<float const> features) const
    {
        if (features.size() != 13 || !std::all_of(features.begin(), features.end(), [](float value) { return std::isfinite(value); }))
            throw std::runtime_error("Invalid target-model input");
        std::array<float, 64> current{}, next{};
        std::copy(features.begin(), features.end(), current.begin());
        for (std::size_t index = 0; index < layers_.size(); ++index)
        {
            auto const& layer = layers_[index];
            for (std::size_t out = 0; out < layer.outputs; ++out)
            {
                float sum = layer.bias[out];
                for (std::size_t in = 0; in < layer.inputs; ++in) sum += current[in]*layer.weights[out*layer.inputs+in];
                next[out] = index < 2 ? std::tanh(sum) : sum;
            }
            current = next;
        }
        if (!std::isfinite(current[0])) throw std::runtime_error("Non-finite target-model score");
        return current[0];
    }

    static Model Parse(std::string_view text, std::string_view expected_checkpoint)
    {
        if (text.size() > 1024*1024) throw std::runtime_error("Target-model artifact too large");
        rapidjson::Document document;
        document.Parse(text.data(), text.size());
        if (document.HasParseError()) throw std::runtime_error("Invalid target-model JSON");
        auto const& schema = Member(document, "schema_version");
        if (!schema.IsUint() || schema.GetUint() != 1) throw std::runtime_error("Target-model schema mismatch");
        auto const& names = Member(document, "feature_names");
        if (!names.IsArray() || names.Size() != 13) throw std::runtime_error("Target-model feature count mismatch");
        for (std::size_t index = 0; index < 13; ++index)
            if (!names[index].IsString() || std::string_view(names[index].GetString()) != FeatureNames[index])
                throw std::runtime_error("Target-model feature order mismatch");
        auto const& identity = Member(document, "checkpoint_sha256");
        if (!identity.IsString()) throw std::runtime_error("Missing target-model checkpoint hash");
        Model model;
        model.checkpoint_sha256 = identity.GetString();
        if (model.checkpoint_sha256.size() != 64 || model.checkpoint_sha256 != expected_checkpoint ||
            !std::all_of(model.checkpoint_sha256.begin(), model.checkpoint_sha256.end(), [](char ch) { return (ch >= '0' && ch <= '9') || (ch >= 'a' && ch <= 'f'); }))
            throw std::runtime_error("Target-model checkpoint hash mismatch");
        auto const& epoch = Member(document, "epoch");
        auto const& step = Member(document, "optimizer_step");
        if (!epoch.IsUint64() || !step.IsUint64() || !epoch.GetUint64() || !step.GetUint64())
            throw std::runtime_error("Missing frozen checkpoint epoch/step");
        model.epoch = epoch.GetUint64(); model.optimizer_step = step.GetUint64();
        auto const& layers = Member(document, "layers");
        if (!layers.IsArray() || layers.Size() != 3) throw std::runtime_error("Target-model layer count mismatch");
        constexpr std::array<std::size_t, 3> Inputs{13, 64, 64}, Outputs{64, 64, 1};
        for (std::size_t index = 0; index < 3; ++index)
        {
            auto& layer = model.layers_[index]; layer.inputs = Inputs[index]; layer.outputs = Outputs[index];
            auto const& weights = Member(layers[index], "weights"); auto const& bias = Member(layers[index], "bias");
            if (!weights.IsArray() || !bias.IsArray() || weights.Size() != layer.outputs || bias.Size() != layer.outputs)
                throw std::runtime_error("Target-model output width mismatch");
            for (auto const& row : weights.GetArray())
            {
                if (!row.IsArray() || row.Size() != layer.inputs) throw std::runtime_error("Target-model input width mismatch");
                for (auto const& value : row.GetArray()) layer.weights.push_back(Number(value));
            }
            for (auto const& value : bias.GetArray()) layer.bias.push_back(Number(value));
        }
        auto const& parity = Member(document, "parity");
        if (!parity.IsArray() || parity.Size() < 2 || parity.Size() > 64) throw std::runtime_error("Missing target-model parity vectors");
        for (auto const& test : parity.GetArray())
        {
            auto const& values = Member(test, "features");
            if (!values.IsArray() || values.Size() != 13) throw std::runtime_error("Invalid target-model parity width");
            Features features{};
            for (std::size_t index = 0; index < 13; ++index) features[index] = Number(values[index]);
            if (std::abs(model.Score(features)-Number(Member(test, "score"))) > 0.0001f)
                throw std::runtime_error("Target-model failed PyTorch parity");
        }
        return model;
    }
};

struct Candidate { std::string guid; Features features; };
struct Frame { std::uint64_t version; std::uint32_t issued_ms; std::vector<Candidate> candidates; };
class History
{
    std::deque<Frame> frames_;
public:
    void Push(Frame frame) { frames_.push_back(std::move(frame)); if (frames_.size() > 64) frames_.pop_front(); }
    Frame const* Find(std::uint64_t version, std::uint32_t now_ms) const
    {
        for (auto it = frames_.rbegin(); it != frames_.rend(); ++it)
            if (it->version == version) return std::uint32_t(now_ms-it->issued_ms) < 2000 ? &*it : nullptr;
        return nullptr;
    }
};
}
