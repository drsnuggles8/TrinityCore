#include "TrainingCapture.h"
#include "LevelingPolicy.h"
#include "TrainingPlayer.h"
#include "TrainingTargetModel.h"
#include "TrainingIntentRules.h"
#include "Bot/BotIntent.h"
#include "Bot/BotIntentEmitter.h"
#include "Bot/BotSnapshot.h"
#include "Config.h"
#include "CryptoHash.h"
#include "GameTime.h"
#include "Log.h"
#include "ObjectMgr.h"
#include "Player.h"
#include "Creature.h"
#include <random>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/writer.h>
#include <boost/lockfree/queue.hpp>
#include <algorithm>
#include <atomic>
#include <chrono>
#include <cmath>
#include <condition_variable>
#include <filesystem>
#include <fstream>
#include <memory>
#include <mutex>
#include <stdexcept>
#include <string>
#include <thread>
#include <unordered_map>

namespace Playerbot::V2::Fleet::TrainingCapture
{
namespace
{
using Buffer = rapidjson::StringBuffer;
using Writer = rapidjson::Writer<Buffer>;
void Text(Writer& w, char const* key, std::string const& value) { w.Key(key); w.String(value.c_str(), static_cast<rapidjson::SizeType>(value.size())); }
void Number(Writer& w, char const* key, double value) { w.Key(key); if (std::isfinite(value)) w.Double(value); else w.Null(); }
std::shared_ptr<TrainingTarget::Model const> shadow_model;
bool control_enabled = false;
std::unordered_set<std::uint64_t> control_guids;
double control_temperature = 1;
struct ControlState
{
    ObjectGuid target;
    std::uint32_t time = 0;
    std::uint64_t decision = 0;
    std::mt19937_64 random;
    explicit ControlState(std::uint64_t guid) : random(guid ^ 0x706c75746full) {}
};
std::mutex control_mutex;
std::unordered_map<std::uint64_t,ControlState> control_states;

TrainingTarget::Features EncodeCandidate(BotSnapshot const& s, NearbyUnit const& enemy)
{
    bool relevant = false;
    for (auto const& quest : s.quest_log.quests)
        for (auto const& objective : quest.objectives)
            if (objective.type == 0 && objective.object_id == static_cast<int32>(enemy.entry) && objective.progress < objective.amount) relevant = true;
    return TrainingTarget::Encode({double(s.vitals.hp),double(s.vitals.max_hp),double(s.vitals.power[POWER_MANA]),
        double(s.vitals.max_power[POWER_MANA]),double(s.identity.level),double(s.bags.bag_free_slots),s.vitals.in_combat},
        {double(enemy.hp),double(enemy.max_hp),double(enemy.level),enemy.x-s.position.x,enemy.y-s.position.y,enemy.z-s.position.z,
         enemy.in_los,enemy.tapped && !enemy.tapped_by_us,relevant});
}

bool Enabled()
{
    static bool const enabled = sConfigMgr->GetBoolDefault("Playerbot.Training.Capture.Enable", false, true);
    return enabled;
}

struct Actor { std::string episode; std::uint64_t deaths = 0, kills = 0; };
struct ShadowState { TrainingTarget::History history; std::string previous_target; };

// The world thread serializes owned values and tries a bounded enqueue. Only this worker touches the output file.
class Sink
{
public:
    Sink()
    {
        capture_ = std::to_string(std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::system_clock::now().time_since_epoch()).count());
        environment_ = sConfigMgr->GetStringDefault("Playerbot.Training.Capture.Environment", "", true);
        if (environment_.empty()) environment_ = "training-" + capture_;
        source_revision_ = sConfigMgr->GetStringDefault("Playerbot.Training.Capture.SourceRevision", "unknown", true);
        realm_ = sConfigMgr->GetIntDefault("RealmID", 0, true);
        max_rows_ = static_cast<std::uint64_t>(std::clamp(sConfigMgr->GetIntDefault("Playerbot.Training.Capture.MaxRows", 10000, true), 100, 1000000));
        max_bytes_ = static_cast<std::uint64_t>(std::clamp(sConfigMgr->GetIntDefault("Playerbot.Training.Capture.MaxMegabytes", 64, true), 1, 2048)) * 1024 * 1024;
        auto directory = std::filesystem::path(sConfigMgr->GetStringDefault("LogsDir", "", true));
        path_ = directory / ("PlutoCapture-" + capture_ + ".jsonl");
        thread_ = std::thread([this] { Run(); });
    }
    ~Sink()
    {
        Stop();
        std::string* pending = nullptr;
        while (queue_.pop(pending)) delete pending;
    }
    bool Accepting() const { return accepting_.load(std::memory_order_relaxed); }
    Actor Context(std::uint64_t guid, bool reset = false, bool died = false, bool killed = false)
    {
        std::lock_guard lock(actors_mutex_);
        auto& actor = actors_[guid];
        if (reset || actor.episode.empty())
        {
            actor = Actor{capture_ + "-" + std::to_string(guid) + "-" + std::to_string(++episode_), 0};
            shadows_.erase(guid);
        }
        if (died) ++actor.deaths;
        if (killed) ++actor.kills;
        return actor;
    }
    void Forget(std::uint64_t guid) { std::lock_guard lock(actors_mutex_); actors_.erase(guid); shadows_.erase(guid); }
    // Both hooks run on the world thread. Store only bounded, owned feature values, never Player/snapshot pointers.
    void Remember(BotSnapshot const& s)
    {
        if (!shadow_model || s.identity.race != 3 || s.identity.cls != 7 || s.identity.level > 6 ||
            s.position.map_id != 0 || s.area.zone_id != 1) return;
        std::lock_guard lock(actors_mutex_);
        if (!shadows_.contains(s.bot_id) && shadows_.size() >= 128) return;
        TrainingTarget::Frame frame{s.version, s.published_at_ms, {}};
        for (auto const& enemy : s.combat.nearby_enemies)
        {
            if (enemy.hp <= 0 || enemy.max_hp <= 0 || enemy.untargetable || enemy.no_xp_kill) continue;
            if (frame.candidates.size() >= 16) break;
            auto features = EncodeCandidate(s,enemy);
            if (!std::all_of(features.begin(), features.end(), [](float value) { return std::isfinite(value); })) return;
            frame.candidates.push_back({enemy.guid.ToString(), features});
        }
        shadows_[s.bot_id].history.Push(std::move(frame));
    }
    void Shadow(Player const* player, Actor const& actor, Intent const& intent, std::uint8_t result, bool exception)
    {
        if (!shadow_model || result != 0 || exception || !Accepting()) return;
        std::lock_guard lock(actors_mutex_);
        auto state_it = shadows_.find(intent.bot_id);
        if (state_it == shadows_.end()) return;
        auto& state = state_it->second;
        if (std::holds_alternative<StopAttackIntent>(intent.body)) { state.previous_target.clear(); return; }
        auto attack = std::get_if<StartAttackIntent>(&intent.body);
        if (!attack) return;
        std::string baseline = attack->target.ToString();
        if (baseline == state.previous_target) return;
        state.previous_target = baseline;
        auto frame = state.history.Find(intent.source_snapshot, GameTime::GetGameTimeMS());
        Buffer buffer; Writer w(buffer); Header(w, "shadow_target", player, actor);
        w.Key("source_snapshot"); w.Uint64(intent.source_snapshot);
        w.Key("intent_id"); w.Uint64(intent.id);
        Text(w, "checkpoint_sha256", shadow_model->checkpoint_sha256);
        w.Key("checkpoint_epoch"); w.Uint64(shadow_model->epoch);
        w.Key("optimizer_step"); w.Uint64(shadow_model->optimizer_step);
        Text(w, "baseline_target", baseline);
        Text(w, "candidate_set", "observed_alive_xp_selectable");
        // This is an observed comparison menu, not a certified executable-action menu. It confers no authority.
        bool baseline_present = frame && std::any_of(frame->candidates.begin(), frame->candidates.end(),
            [&](auto const& candidate) { return candidate.guid == baseline; });
        char const* reason = !frame ? "missing_or_stale_source" : frame->candidates.size() < 2 ? "no_actual_choice" :
            !baseline_present ? "baseline_not_observed" : "scored";
        Text(w, "status", reason);
        if (std::string_view(reason) == "scored")
        {
            auto start = std::chrono::steady_clock::now();
            std::vector<float> scores;
            scores.reserve(frame->candidates.size());
            for (auto const& candidate : frame->candidates) scores.push_back(shadow_model->Score(candidate.features));
            auto index = static_cast<std::size_t>(std::max_element(scores.begin(), scores.end())-scores.begin());
            auto nearest = std::min_element(frame->candidates.begin(), frame->candidates.end(),
                [](auto const& a, auto const& b) { return a.features[8] < b.features[8]; });
            auto elapsed = std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::steady_clock::now()-start).count();
            w.Key("inference_us"); w.Double(double(elapsed)/1000);
            Text(w, "proposed_target", frame->candidates[index].guid);
            Text(w, "nearest_target", nearest->guid);
            w.Key("candidates"); w.StartArray();
            for (std::size_t i = 0; i < scores.size(); ++i)
            {
                w.StartObject(); Text(w, "guid", frame->candidates[i].guid);
                w.Key("features"); w.StartArray(); for (float value : frame->candidates[i].features) w.Double(value); w.EndArray();
                w.Key("score"); w.Double(scores[i]); w.EndObject();
            }
            w.EndArray();
        }
        w.EndObject(); Push(buffer);
    }
    void Header(Writer& w, char const* kind, Player const* player, Actor const& actor)
    { HeaderForBot(w,kind,player->GetGUID().GetCounter(),actor); }
    void HeaderForBot(Writer& w, char const* kind, std::uint64_t guid, Actor const& actor)
    {
        w.StartObject();
        w.Key("schema_version"); w.Uint(1);
        Text(w, "kind", kind); Text(w, "capture_id", capture_); Text(w, "environment_id", environment_);
        Text(w, "source_revision", source_revision_);
        Text(w, "episode_id", actor.episode); Text(w, "actor_guid", std::to_string(guid));
        w.Key("realm_id"); w.Int(realm_);
        w.Key("recorded_at_ms"); w.Uint(GameTime::GetGameTimeMS());
        bool leveling = LevelingPolicy::Controls(guid);
        bool controlled = ControlsQuestTarget(guid) || leveling;
        Text(w, "behavior_policy", leveling ? "learned-leveling-options-v1" : controlled ? "learned-quest-target-v1" : "scripted-v2");
        w.Key("model_authority"); w.Bool(controlled);
        w.Key("death_count"); w.Uint64(actor.deaths);
        w.Key("monster_killing_blows"); w.Uint64(actor.kills);
        w.Key("dropped_before"); w.Uint64(dropped_.load(std::memory_order_relaxed));
    }
    void Push(Buffer const& buffer)
    {
        if (!Accepting()) return;
        active_producers_.fetch_add(1, std::memory_order_acq_rel);
        struct ProducerGuard
        {
            std::atomic<std::uint64_t>& count;
            std::condition_variable& ready;
            ~ProducerGuard() { count.fetch_sub(1, std::memory_order_acq_rel); ready.notify_one(); }
        } guard{active_producers_, ready_};
        if (!Accepting()) return;
        // Policy decisions can be produced on bot workers. Reserve before enqueue so parallel producers cannot overshoot.
        auto slot = enqueued_.fetch_add(1,std::memory_order_acq_rel);
        if (slot >= max_rows_)
        {
            accepting_.store(false); budget_.store(true); stopping_.store(true); ready_.notify_one();
            return;
        }
        auto line = std::make_unique<std::string>(buffer.GetString(), buffer.GetSize());
        if (!queue_.push(line.get()))
        {
            dropped_.fetch_add(1, std::memory_order_relaxed);
            if (slot+1 >= max_rows_) { accepting_.store(false); budget_.store(true); stopping_.store(true); }
            return;
        }
        line.release();
        if (slot + 1 >= max_rows_)
        {
            accepting_.store(false, std::memory_order_relaxed);
            budget_.store(true, std::memory_order_release);
            stopping_.store(true, std::memory_order_release);
        }
        ready_.notify_one();
    }
    void Stop()
    {
        accepting_.store(false, std::memory_order_relaxed);
        stopping_.store(true, std::memory_order_release);
        ready_.notify_one();
        if (thread_.joinable()) thread_.join();
    }
    void Abort()
    {
        accepting_.store(false, std::memory_order_relaxed);
        error_.store(true, std::memory_order_release);
        stopping_.store(true, std::memory_order_release);
        ready_.notify_one();
    }
private:
    void Run() noexcept
    {
        try
        {
            if (!path_.parent_path().empty()) std::filesystem::create_directories(path_.parent_path());
            std::ofstream output(path_, std::ios::out | std::ios::app);
            if (!output) throw std::runtime_error("capture output could not be opened");
            TC_LOG_INFO("playerbot.v2", "[training_capture] writing bounded observation-only capture to {}", path_.string());
            std::uint64_t rows = 0, bytes = 0;
            std::string reason = "shutdown";
            while (true)
            {
                std::string* pending = nullptr;
                if (!queue_.pop(pending))
                {
                    if (stopping_.load(std::memory_order_acquire) && active_producers_.load(std::memory_order_acquire) == 0)
                    {
                        // A producer can finish between the first empty pop and the count read.
                        if (queue_.pop(pending)) { /* consume its final row below */ }
                        else
                        {
                            if (error_.load(std::memory_order_acquire)) reason = "capture_error";
                            else if (budget_.load(std::memory_order_acquire)) reason = "budget";
                            break;
                        }
                    }
                    else
                    {
                        std::unique_lock lock(wait_mutex_);
                        // Producers never wait on this mutex; timeout also covers a notification before wait starts.
                        ready_.wait_for(lock, std::chrono::milliseconds(10));
                        continue;
                    }
                }
                std::unique_ptr<std::string> line(pending);
                if (rows >= max_rows_ || bytes + line->size() + 1 > max_bytes_)
                {
                    accepting_.store(false, std::memory_order_relaxed);
                    budget_.store(true, std::memory_order_release);
                    stopping_.store(true, std::memory_order_release);
                    dropped_.fetch_add(1, std::memory_order_relaxed);
                    continue;
                }
                output << *line << '\n';
                if (!output) throw std::runtime_error("capture write failed");
                ++rows; bytes += line->size() + 1;
            }
            Buffer end; Writer w(end); w.StartObject(); Text(w, "kind", "capture_end");
            Text(w, "capture_id", capture_); Text(w, "reason", reason);
            w.Key("rows"); w.Uint64(rows); w.Key("dropped_total"); w.Uint64(dropped_.load()); w.EndObject();
            output << end.GetString() << '\n'; output.flush();
            TC_LOG_INFO("playerbot.v2", "[training_capture] finished rows={} bytes={} dropped={} reason={}", rows, bytes, dropped_.load(), reason);
        }
        catch (std::exception const& error)
        {
            accepting_.store(false, std::memory_order_relaxed);
            TC_LOG_ERROR("playerbot.v2", "[training_capture] disabled: {}", error.what());
        }
    }
    std::atomic<bool> accepting_{true};
    std::atomic<std::uint64_t> dropped_{0};
    std::mutex wait_mutex_, actors_mutex_;
    std::condition_variable ready_;
    // A hundred freshly logged-in players can publish observations, quest and policy events in one burst.
    // Keep producer enqueue non-blocking while bounding the backlog to a few tens of MiB in ordinary captures.
    boost::lockfree::queue<std::string*, boost::lockfree::capacity<4096>> queue_;
    std::unordered_map<std::uint64_t, Actor> actors_;
    std::unordered_map<std::uint64_t, ShadowState> shadows_;
    std::atomic<bool> stopping_{false}, error_{false}, budget_{false};
    std::atomic<std::uint64_t> active_producers_{0}, enqueued_{0};
    std::thread thread_;
    std::filesystem::path path_;
    std::string capture_, environment_, source_revision_;
    std::uint64_t episode_ = 0, max_rows_ = 0, max_bytes_ = 0;
    int realm_ = 0;
};

std::mutex sink_mutex;
std::shared_ptr<Sink> sink;
std::atomic<bool> shutting_down{false}, failed{false};
template<class Function> void Safely(Function&& operation) noexcept
{
    try { operation(); }
    catch (...)
    {
        failed.store(true, std::memory_order_relaxed);
        std::shared_ptr<Sink> old;
        { std::lock_guard lock(sink_mutex); old = sink; }
        if (old) old->Abort();
    }
}
std::shared_ptr<Sink> Get(Player const* player)
{
    if (shutting_down.load(std::memory_order_relaxed) || failed.load(std::memory_order_relaxed) || !Enabled() || !IsTrainingPlayer(player)) return {};
    std::lock_guard lock(sink_mutex);
    if (shutting_down.load(std::memory_order_relaxed) || failed.load(std::memory_order_relaxed)) return {};
    if (!sink) sink = std::make_shared<Sink>();
    return sink->Accepting() ? sink : nullptr;
}
std::shared_ptr<Sink> GetForBot(std::uint64_t guid)
{
    if (!(ControlsQuestTarget(guid) || LevelingPolicy::Controls(guid)) || failed.load() || shutting_down.load()) return {};
    std::lock_guard lock(sink_mutex);
    return sink && sink->Accepting() ? sink : nullptr;
}
void Event(Player const* player, char const* kind, bool reset = false, bool died = false)
{
    auto output = Get(player); if (!output) return;
    auto actor = output->Context(player->GetGUID().GetCounter(), reset, died);
    Buffer buffer; Writer w(buffer); output->Header(w, kind, player, actor); w.EndObject(); output->Push(buffer);
}
}

void Initialize()
{
    if (!Enabled()) return;
    LevelingPolicy::Initialize();
    if (!sConfigMgr->GetBoolDefault("Playerbot.Training.TargetShadow.Enable", false, true)) return;
    try
    {
        std::string path = sConfigMgr->GetStringDefault("Playerbot.Training.TargetShadow.ModelFile", "", true);
        std::string expected = sConfigMgr->GetStringDefault("Playerbot.Training.TargetShadow.CheckpointSHA256", "", true);
        std::string artifact = sConfigMgr->GetStringDefault("Playerbot.Training.TargetShadow.ArtifactSHA256", "", true);
        if (std::filesystem::file_size(path) > 1024*1024) throw std::runtime_error("Model artifact exceeds 1 MiB");
        std::ifstream input(path, std::ios::binary);
        if (!input) throw std::runtime_error("Model artifact could not be opened");
        std::string text((std::istreambuf_iterator<char>(input)), std::istreambuf_iterator<char>());
        std::string actual;
        constexpr char Hex[] = "0123456789abcdef";
        for (auto byte : Trinity::Crypto::SHA256::GetDigestOf(text)) { actual += Hex[byte >> 4]; actual += Hex[byte & 15]; }
        if (artifact.size() != 64 || actual != artifact) throw std::runtime_error("Model artifact SHA256 mismatch");
        shadow_model = std::make_shared<TrainingTarget::Model const>(TrainingTarget::Model::Parse(text, expected));
        control_enabled = sConfigMgr->GetBoolDefault("Playerbot.Training.TargetControl.Enable",false,true);
        control_guids = ParseTrainingPlayerGuids(sConfigMgr->GetStringDefault("Playerbot.Training.TargetControl.PlayerGuids","",true));
        control_temperature = std::clamp(double(sConfigMgr->GetFloatDefault("Playerbot.Training.TargetControl.Temperature",1.f,true)),0.05,5.0);
        TC_LOG_INFO("playerbot.v2", "[training_shadow] loaded checkpoint={} epoch={} step={} parity=passed authority={}",
            shadow_model->checkpoint_sha256, shadow_model->epoch, shadow_model->optimizer_step,
            control_enabled && !control_guids.empty() ? "quest_targets" : "none");
        TC_LOG_INFO("playerbot.v2","[training_control] enabled={} configured_actors={} temperature={}",control_enabled,control_guids.size(),control_temperature);
    }
    catch (std::exception const& error)
    {
        shadow_model.reset();
        TC_LOG_ERROR("playerbot.v2", "[training_shadow] disabled; scripted AI retained: {}", error.what());
    }
}

bool ControlsQuestTarget(std::uint64_t guid)
{ return control_enabled && shadow_model && IsTrainingPlayerGuid(guid) && control_guids.contains(guid); }

bool PolicyRecordingAvailable(std::uint64_t guid) { return bool(GetForBot(guid)); }
bool PolicyEvent(std::uint64_t guid,std::uint64_t source,char const* kind,std::string const& payload)
{
    auto output=GetForBot(guid);if (!output) return false;
    auto actor=output->Context(guid);Buffer buffer;Writer w(buffer);output->HeaderForBot(w,kind,guid,actor);
    w.Key("source_snapshot");w.Uint64(source);w.Key("leveling");w.RawValue(payload.c_str(),payload.size(),rapidjson::kObjectType);
    w.EndObject();output->Push(buffer);return true;
}

NearbyUnit const* TryQuestTarget(BotSnapshot const& s, std::span<NearbyUnit const* const> menu, BotIntentEmitter& emitter)
{
    auto output = GetForBot(s.bot_id);
    if (!output || menu.empty() || menu.size() > 16 || s.identity.race != 3 || s.identity.cls != 7 ||
        s.identity.level > 6 || !s.vitals.is_alive || s.vitals.in_combat || s.position.map_id != 0 || s.area.zone_id != 1 ||
        std::uint32_t(GameTime::GetGameTimeMS()-s.published_at_ms) >= 2000) return nullptr;
    try
    {
        std::lock_guard lock(control_mutex);
        if (!control_states.contains(s.bot_id) && control_states.size() >= 128) return nullptr;
        auto& state = control_states.try_emplace(s.bot_id,s.bot_id).first->second;
        if (state.time && std::uint32_t(s.published_at_ms-state.time) < 10000)
            for (auto candidate : menu)
                if (candidate->guid == state.target)
                    return emitter.start_attack(candidate->guid) ? candidate : nullptr;
        if (menu.size() < 2) return nullptr;
        std::vector<TrainingTarget::Features> encoded;
        std::vector<float> scores;
        for (auto candidate : menu) { encoded.push_back(EncodeCandidate(s,*candidate)); scores.push_back(shadow_model->Score(encoded.back())); }
        auto sampled = TrainingTarget::Sample(scores,control_temperature,std::generate_canonical<double,53>(state.random));
        auto target = menu[sampled.index];
        bool emitted = emitter.start_attack(target->guid);
        auto actor = output->Context(s.bot_id);
        Buffer buffer; Writer w(buffer); output->HeaderForBot(w,"policy_target",s.bot_id,actor);
        w.Key("source_snapshot"); w.Uint64(s.version); w.Key("decision_id"); w.Uint64(++state.decision);
        Text(w,"checkpoint_sha256",shadow_model->checkpoint_sha256);
        w.Key("checkpoint_epoch"); w.Uint64(shadow_model->epoch); w.Key("optimizer_step"); w.Uint64(shadow_model->optimizer_step);
        w.Key("temperature"); w.Double(control_temperature); w.Key("selected_index"); w.Uint64(sampled.index);
        Text(w,"selected_target",target->guid.ToString()); Text(w,"baseline_target",menu.front()->guid.ToString());
        w.Key("emitted"); w.Bool(emitted); w.Key("log_probability"); w.Double(std::log(std::max(1e-30,sampled.probabilities[sampled.index])));
        w.Key("candidates"); w.StartArray();
        for (std::size_t i = 0; i < menu.size(); ++i)
        {
            w.StartObject(); Text(w,"guid",menu[i]->guid.ToString());
            w.Key("features"); w.StartArray(); for (float value : encoded[i]) w.Double(value); w.EndArray();
            w.Key("score"); w.Double(scores[i]); w.Key("probability"); w.Double(sampled.probabilities[i]); w.EndObject();
        }
        w.EndArray(); w.EndObject(); output->Push(buffer);
        if (!emitted) return nullptr;
        state.target = target->guid; state.time = s.published_at_ms;
        return target;
    }
    catch (...) { return nullptr; } // fall back to the existing quest dispatcher immediately
}

void Login(Player const* player)
{
    Safely([&] { Event(player,"login",true); });
    if (player && IsTrainingPlayer(player)) { std::lock_guard lock(control_mutex); control_states.erase(player->GetGUID().GetCounter()); }
}
void Logout(Player const* player)
{
    if (player) LevelingPolicy::Logout(player->GetGUID().GetCounter());
    Safely([&] {
        Event(player, "logout");
        auto output = Get(player); if (output) output->Forget(player->GetGUID().GetCounter());
    });
    if (player && IsTrainingPlayer(player)) { std::lock_guard lock(control_mutex); control_states.erase(player->GetGUID().GetCounter()); }
}
void Death(Player const* player)
{
    Safely([&] { Event(player,"death",false,true); });
    if (player && IsTrainingPlayer(player)) { std::lock_guard lock(control_mutex); control_states.erase(player->GetGUID().GetCounter()); }
}
void MonsterKill(Player const* player, Creature const* creature)
{
    Safely([&] {
        auto output = Get(player); if (!output || !creature) return;
        auto actor = output->Context(player->GetGUID().GetCounter(),false,false,true);
        Buffer buffer; Writer w(buffer); output->Header(w,"monster_kill",player,actor);
        Text(w,"creature_guid",creature->GetGUID().ToString()); w.Key("creature_entry"); w.Uint(creature->GetEntry());
        w.Key("creature_level"); w.Uint(creature->GetLevel()); Text(w,"attribution","killing_blow");
        w.EndObject(); output->Push(buffer);
    });
}

void ObservationImpl(Player const* player, BotSnapshot const& s)
{
    auto output = Get(player); if (!output) return;
    auto actor = output->Context(s.bot_id);
    Buffer buffer; Writer w(buffer); output->Header(w, "observation", player, actor);
    w.Key("snapshot_version"); w.Uint64(s.version);
    w.Key("world_tick"); w.Uint64(s.world_tick);
    w.Key("observed_at_ms"); w.Uint(s.published_at_ms);
    w.Key("self"); w.StartObject();
    w.Key("level"); w.Uint(s.identity.level); w.Key("race"); w.Uint(s.identity.race); w.Key("class"); w.Uint(s.identity.cls);
    w.Key("xp"); w.Uint(s.identity.xp); w.Key("xp_for_level"); w.Uint(s.identity.xp_for_level);
    std::uint64_t cumulative = s.identity.xp;
    for (std::uint8_t level = 1; level < s.identity.level; ++level) cumulative += sObjectMgr->GetXPForLevel(level);
    w.Key("xp_total_coordinate"); w.Uint64(cumulative); w.Key("xp_reference"); w.Uint(sObjectMgr->GetXPForLevel(1));
    w.Key("health"); w.Int(s.vitals.hp); w.Key("max_health"); w.Int(s.vitals.max_hp);
    w.Key("mana"); w.Int(s.vitals.power[POWER_MANA]); w.Key("max_mana"); w.Int(s.vitals.max_power[POWER_MANA]);
    w.Key("alive"); w.Bool(s.vitals.is_alive); w.Key("in_combat"); w.Bool(s.vitals.in_combat);
    Text(w,"selected_target",s.combat.current_target.ToString()); Text(w,"victim",s.combat.victim.ToString());
    w.Key("path_blocked_count"); w.Uint(s.path_telemetry.count);
    w.Key("map"); w.Uint(s.position.map_id); w.Key("instance"); w.Uint(s.position.instance_id); w.Key("zone"); w.Uint(s.area.zone_id);
    Number(w, "x", s.position.x); Number(w, "y", s.position.y); Number(w, "z", s.position.z); Number(w, "orientation", s.position.o);
    w.Key("moving"); w.Bool(s.movement.is_moving); w.Key("money_copper"); w.Int(s.inventory.gold);
    w.Key("bag_free_slots"); w.Uint(s.bags.bag_free_slots);
    w.Key("known_spells"); w.StartArray(); for (auto spell : s.spellbook.known_spells) w.Uint(spell); w.EndArray();
    w.EndObject();
    w.Key("rewarded_quests"); w.StartArray(); for (auto quest : player->getRewardedQuests()) w.Uint(quest); w.EndArray();
    w.Key("quests"); w.StartArray();
    for (auto const& quest : s.quest_log.quests)
    {
        w.StartObject(); w.Key("id"); w.Uint(quest.quest_id); w.Key("state"); w.Uint(quest.state);
        w.Key("xp_reward"); w.Uint(quest.xp_reward); w.Key("flags"); w.Uint(quest.flags);
        w.Key("unturnable"); w.Bool(quest.unturnable); w.Key("unachievable"); w.Bool(quest.unachievable);
        w.Key("objectives"); w.StartArray();
        for (auto const& objective : quest.objectives)
        {
            w.StartObject(); w.Key("id"); w.Uint(objective.id); w.Key("type"); w.Uint(objective.type);
            w.Key("object_id"); w.Int(objective.object_id); w.Key("required"); w.Int(objective.amount); w.Key("progress"); w.Int(objective.progress); w.EndObject();
        }
        w.EndArray(); w.EndObject();
    }
    w.EndArray(); w.Key("enemies"); w.StartArray();
    for (auto const& enemy : s.combat.nearby_enemies)
    {
        w.StartObject(); Text(w, "guid", enemy.guid.ToString()); w.Key("entry"); w.Uint(enemy.entry);
        w.Key("level"); w.Uint(enemy.level); w.Key("health"); w.Int(enemy.hp); w.Key("max_health"); w.Int(enemy.max_hp);
        Number(w, "dx", enemy.x - s.position.x); Number(w, "dy", enemy.y - s.position.y); Number(w, "dz", enemy.z - s.position.z);
        w.Key("in_los"); w.Bool(enemy.in_los); w.Key("tapped"); w.Bool(enemy.tapped); w.Key("tapped_by_us"); w.Bool(enemy.tapped_by_us);
        w.Key("no_xp"); w.Bool(enemy.no_xp_kill); w.Key("unselectable"); w.Bool(enemy.untargetable); w.EndObject();
    }
    w.EndArray(); w.EndObject(); output->Push(buffer);
    output->Remember(s);
    LevelingPolicy::Observe(player,s,cumulative,actor.deaths,sObjectMgr->GetXPForLevel(1));
}

void ExecutedImpl(Player const* player, Intent const& intent, std::uint8_t result, bool exception)
{
    auto output = Get(player); if (!output) return;
    auto actor = output->Context(intent.bot_id);
    Buffer buffer; Writer w(buffer); output->Header(w, "action_attempt", player, actor);
    w.Key("intent_id"); w.Uint64(intent.id); w.Key("source_snapshot"); w.Uint64(intent.source_snapshot);
    Text(w, "action_kind", IntentKindName(intent.body.index()));
    w.Key("result"); w.Uint(result); w.Key("exception"); w.Bool(exception);
    w.Key("accepted_by_api"); w.Bool(result == 0 && !exception);
    w.Key("privileged_action"); w.Bool(IsPrivilegedTrainingIntent(intent.body));
    bool payload = true;
    w.Key("payload"); w.StartObject();
    if (auto const* value = std::get_if<MoveToIntent>(&intent.body))
    { Number(w, "x", value->x); Number(w, "y", value->y); Number(w, "z", value->z); w.Key("run"); w.Bool(value->run); w.Key("direct"); w.Bool(value->direct); }
    else if (auto const* value = std::get_if<StartAttackIntent>(&intent.body)) Text(w, "target", value->target.ToString());
    else if (auto const* value = std::get_if<CastSpellIntent>(&intent.body))
    { Text(w, "target", value->target.ToString()); w.Key("spell_id"); w.Uint(value->spell_id); }
    else if (auto const* value = std::get_if<LootIntent>(&intent.body)) Text(w, "target", value->corpse_or_object.ToString());
    else if (auto const* value = std::get_if<QuestAcceptIntent>(&intent.body))
    { Text(w, "target", value->npc.ToString()); w.Key("quest_id"); w.Uint(value->quest_id); }
    else if (auto const* value = std::get_if<QuestCompleteIntent>(&intent.body))
    { Text(w, "target", value->npc.ToString()); w.Key("quest_id"); w.Uint(value->quest_id); w.Key("reward_choice"); w.Uint(value->reward_choice); }
    else payload = false;
    w.EndObject(); w.Key("payload_captured"); w.Bool(payload); w.EndObject(); output->Push(buffer);
    LevelingPolicy::Executed(intent.bot_id,intent.source_snapshot,exception?255:result);
    output->Shadow(player, actor, intent, result, exception);
}

void Observation(Player const* player, BotSnapshot const& snapshot)
{
    Safely([&] { ObservationImpl(player, snapshot); });
}
void Executed(Player const* player, Intent const& intent, std::uint8_t result, bool exception)
{
    Safely([&] { ExecutedImpl(player, intent, result, exception); });
}
void Shutdown()
{
    shutting_down.store(true, std::memory_order_relaxed);
    std::shared_ptr<Sink> old;
    { std::lock_guard lock(sink_mutex); old = std::move(sink); }
    if (old) old->Stop();
}
}
