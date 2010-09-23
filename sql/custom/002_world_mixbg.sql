DELETE FROM `trinity_string` WHERE entry IN (12001,12002,12003,12004);
INSERT INTO `trinity_string` VALUES (12001, 'Queue status for %s (Lvl: %u to %u)\nQueued: %u (Need at least %u more)\n', NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL);
INSERT INTO `trinity_string` VALUES (12002, 'Mix Battleground Mode ACTIVATED', NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL);
INSERT INTO `trinity_string` VALUES (12003, 'Mix Battleground Mode DEACTIVATED', NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL);
INSERT INTO `trinity_string` VALUES (12004, '|cffff0000[BG Queue Announcer]:|r %s -- [%u-%u] Queued/Need: %u/%u', NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL);

UPDATE gameobject_template SET faction = 83 WHERE entry in (180059, 180058);
UPDATE gameobject_template SET faction = 84 WHERE entry in (180060,180061);
UPDATE creature_template SET faction_A = 84, faction_H = 84 WHERE entry IN (13116,22526,31920,37236);
UPDATE creature_template SET faction_A = 83, faction_H = 83 WHERE entry IN (13117,22558,32004,37323);
