-- Shattered Barrier
DELETE FROM `spell_proc_event` WHERE `entry` IN (44745, 54787, 58426, 31221, 31222, 31223);
INSERT INTO `spell_proc_event` VALUES
(44745, 0x00, 3, 0x00000000, 0x00000001, 0x00000000, 0x00008000, 0x0002000, 0.000000, 0.000000, 0),
(54787, 0x00, 3, 0x00000000, 0x00000001, 0x00000000, 0x00008000, 0x0002000, 0.000000, 0.000000, 0);