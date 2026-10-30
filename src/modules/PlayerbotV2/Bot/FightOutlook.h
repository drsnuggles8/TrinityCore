#pragma once
// Whether a solo bot low on health is winning its fight, at the pace of the last few seconds: the bot's health and the summed
// health of its attackers, sampled a few times a second (combat:solo_flee, State_InCombat). A bot that would kill its mobs well
// before they kill it fights on instead of running (Olo, 30 Oct: in build76 guided60 a third of the flees ended in death, most
// in the Coldridge troll cave, where running pulls more whelps; one bot ran at 22 % health from a single whelp at 59 %).
// Pure: tested in tests/game/ClassicFightOutlook.cpp.

#include <array>
#include <cstdint>

namespace Playerbot::FightOutlook {

constexpr std::uint32_t kMinWindowMs = 1500;   // a pace needs at least this much of the fight ...
constexpr std::uint32_t kMaxWindowMs = 4000;   // ... and is read over at most this much
constexpr std::uint32_t kSampleEveryMs = 500;
constexpr float kMargin = 1.5f;                 // the mobs must be dead this many times sooner than the bot would be

struct Sample
{
    std::uint32_t at_ms = 0;
    std::int32_t own_hp = 0;
    std::int32_t mobs_hp = 0;   // the attackers' health, summed
};

// The pace between `then` and `now`: winning when the bot is not losing health at all, or when at that pace its attackers die
// kMargin times sooner than it does. No pace yet (window under kMinWindowMs) or no damage dealt: not winning, the bot flees as
// before. A mob joining the fight raises the summed health, so the pace turns against the bot.
constexpr bool Winning(Sample const& then, Sample const& now)
{
    if (now.at_ms - then.at_ms < kMinWindowMs)
        return false;
    std::int64_t const lost = std::int64_t(then.own_hp) - now.own_hp;
    std::int64_t const dealt = std::int64_t(then.mobs_hp) - now.mobs_hp;
    if (lost <= 0)
        return true;
    if (dealt <= 0)
        return false;
    // time to die = own_hp * dt / lost, time to kill = mobs_hp * dt / dealt
    return double(now.own_hp) * double(dealt) > kMargin * double(now.mobs_hp) * double(lost);
}

// A bot losing at a pace that kills it within kRunWhenDeadInMs runs at up to kRunEarlyBelowPct health instead of waiting for its
// class threshold (25-35 %). guided30-b85: 9 of 12 flees begun under 15 % health ended in death, 3 of 56 begun at 25 % or more;
// a level 2 paladin (about 60 hp) under two Defias Cutpurses had under 2 s left at its 25 %.
constexpr std::uint32_t kNoEnd = 0xFFFFFFFFu;
constexpr std::uint32_t kRunWhenDeadInMs = 6000;
constexpr int kRunEarlyBelowPct = 50;

// The time the bot has left at the pace between `then` and `now`: kNoEnd when it is not losing health or there is no pace yet.
constexpr std::uint32_t MsToDie(Sample const& then, Sample const& now)
{
    if (now.at_ms - then.at_ms < kMinWindowMs)
        return kNoEnd;
    std::int64_t const lost = std::int64_t(then.own_hp) - now.own_hp;
    if (lost <= 0)
        return kNoEnd;
    std::int64_t const ms = std::int64_t(now.own_hp > 0 ? now.own_hp : 0) * std::int64_t(now.at_ms - then.at_ms) / lost;
    return ms >= std::int64_t(kNoEnd) ? kNoEnd : std::uint32_t(ms);
}

// The last samples of a fight, kSampleEveryMs apart.
struct History
{
    std::array<Sample, 10> ring{};
    std::uint8_t count = 0, head = 0;   // head: the next slot written

    void Add(Sample const& s)
    {
        // A gap means a new fight: the old pace says nothing about it.
        if (count && s.at_ms - Newest().at_ms > kMaxWindowMs)
            count = 0;
        if (count && s.at_ms - Newest().at_ms < kSampleEveryMs)
            return;
        ring[head] = s;
        head = std::uint8_t((head + 1) % ring.size());
        if (count < ring.size())
            ++count;
    }
    Sample const& Newest() const { return ring[(head + ring.size() - 1) % ring.size()]; }
    // The oldest sample at most kMaxWindowMs before `now_ms`; nullptr when there is none.
    Sample const* Reference(std::uint32_t now_ms) const
    {
        for (std::uint8_t i = count; i > 0; --i)
        {
            Sample const& s = ring[(head + ring.size() - i) % ring.size()];
            if (now_ms - s.at_ms <= kMaxWindowMs)
                return &s;
        }
        return nullptr;
    }
};

} // namespace Playerbot::FightOutlook
