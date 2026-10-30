-- Grik'nir the Cold (808), The Stolen Journal: on WoW Forever he is back 1-2 s after he dies, so a crowd at the Coldridge troll
-- cave takes turns on him. Here he had 60 s in the dynamic spawn group, kept in full while few players stood at his spot (Olo,
-- 30 Oct). 2 s is below Respawn.DynamicMinimum.Creature, so the dynamic rate leaves it alone, and below Respawn.CombatWaitMax,
-- so he does not wait for the fights around him either (Map::CheckRespawn).
UPDATE `creature` SET `spawntimesecs`=2 WHERE `id`=808 AND `guid`=20001502;
