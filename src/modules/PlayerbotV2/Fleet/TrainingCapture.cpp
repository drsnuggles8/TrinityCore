#include "TrainingCapture.h"
#include "TrainingPlayer.h"
#include "Bot/BotIntent.h"
#include "Bot/BotSnapshot.h"
#include "Config.h"
#include "GameTime.h"
#include "Log.h"
#include "ObjectMgr.h"
#include "Player.h"
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

bool Enabled()
{
    static bool const enabled = sConfigMgr->GetBoolDefault("Playerbot.Training.Capture.Enable", false, true);
    return enabled;
}

struct Actor { std::string episode; std::uint64_t deaths = 0; };

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
        max_bytes_ = static_cast<std::uint64_t>(std::clamp(sConfigMgr->GetIntDefault("Playerbot.Training.Capture.MaxMegabytes", 64, true), 1, 1024)) * 1024 * 1024;
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
    Actor Context(std::uint64_t guid, bool reset = false, bool died = false)
    {
        std::lock_guard lock(actors_mutex_);
        auto& actor = actors_[guid];
        if (reset || actor.episode.empty()) actor = Actor{capture_ + "-" + std::to_string(guid) + "-" + std::to_string(++episode_), 0};
        if (died) ++actor.deaths;
        return actor;
    }
    void Forget(std::uint64_t guid) { std::lock_guard lock(actors_mutex_); actors_.erase(guid); }
    void Header(Writer& w, char const* kind, Player const* player, Actor const& actor)
    {
        w.StartObject();
        w.Key("schema_version"); w.Uint(1);
        Text(w, "kind", kind); Text(w, "capture_id", capture_); Text(w, "environment_id", environment_);
        Text(w, "source_revision", source_revision_);
        Text(w, "episode_id", actor.episode); Text(w, "actor_guid", std::to_string(player->GetGUID().GetCounter()));
        w.Key("realm_id"); w.Int(realm_);
        w.Key("recorded_at_ms"); w.Uint(GameTime::GetGameTimeMS());
        Text(w, "behavior_policy", "scripted-v2");
        w.Key("model_authority"); w.Bool(false);
        w.Key("death_count"); w.Uint64(actor.deaths);
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
        auto line = std::make_unique<std::string>(buffer.GetString(), buffer.GetSize());
        if (!queue_.push(line.get()))
        {
            dropped_.fetch_add(1, std::memory_order_relaxed);
            return;
        }
        line.release();
        if (enqueued_.fetch_add(1, std::memory_order_relaxed) + 1 >= max_rows_)
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
    boost::lockfree::queue<std::string*, boost::lockfree::capacity<512>> queue_;
    std::unordered_map<std::uint64_t, Actor> actors_;
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
void Event(Player const* player, char const* kind, bool reset = false, bool died = false)
{
    auto output = Get(player); if (!output) return;
    auto actor = output->Context(player->GetGUID().GetCounter(), reset, died);
    Buffer buffer; Writer w(buffer); output->Header(w, kind, player, actor); w.EndObject(); output->Push(buffer);
}
}

void Login(Player const* player) { Safely([&] { Event(player, "login", true); }); }
void Logout(Player const* player)
{
    Safely([&] {
        Event(player, "logout");
        auto output = Get(player); if (output) output->Forget(player->GetGUID().GetCounter());
    });
}
void Death(Player const* player) { Safely([&] { Event(player, "death", false, true); }); }

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
    w.Key("privileged_action"); w.Bool(std::holds_alternative<UnstuckIntent>(intent.body) || std::holds_alternative<TeleportToIntent>(intent.body)
        || std::holds_alternative<NearTeleportToIntent>(intent.body) || std::holds_alternative<ResetCooldownsIntent>(intent.body));
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
