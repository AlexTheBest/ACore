DELETE FROM `trinity_string` WHERE entry IN (12001,12002,12003,12004);
INSERT INTO `trinity_string` VALUES (12001, 'Queue status for %s (Lvl: %u to %u)\nQueued: %u (Need at least %u more)\n', NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL);
INSERT INTO `trinity_string` VALUES (12002, 'Mix Battleground Mode ACTIVATED', NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL);
INSERT INTO `trinity_string` VALUES (12003, 'Mix Battleground Mode DEACTIVATED', NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL);
INSERT INTO `trinity_string` VALUES (12004, '|cffff0000[BG Queue Announcer]:|r %s -- [%u-%u] Queued/Need: %u/%u', NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL);
