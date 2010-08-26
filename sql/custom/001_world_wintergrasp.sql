ALTER TABLE db_version CHANGE COLUMN required_436_world_command required_507_world_wintergrasp bit;

DELETE FROM `trinity_string` WHERE entry IN (756,757,758,759,760,761,762,763,764,765,766,767,768,769,770,771,772,780,781);
INSERT INTO `trinity_string` VALUES ('756', 'Battle begins!', '', '', '', '', '', '', '', 'Битва началась');
INSERT INTO `trinity_string` VALUES ('757', '%s has successfully defended the fortress!', '', '', '', '', '', '', '', '');
INSERT INTO `trinity_string` VALUES ('758', '%s has taken over the fortress!', '', '', '', '', '', '', '', '');
INSERT INTO `trinity_string` VALUES ('759', 'The %s siege workshop has been damaged by the %s!', '', '', '', '', '', '', '', '');
INSERT INTO `trinity_string` VALUES ('760', 'The %s siege workshop has been destroyed by the %s!', '', '', '', '', '', '', '', '');
INSERT INTO `trinity_string` VALUES ('761', 'The %s tower has been damaged!', '', '', '', '', '', '', '', '%s башня повреждена');
INSERT INTO `trinity_string` VALUES ('762', 'The %s tower has been destroyed!', '', '', '', '', '', '', '', '%s башня уничтожена!');
INSERT INTO `trinity_string` VALUES ('763', 'Wintergrasp fortress is under attack!', '', '', '', '', '', '', '', '');
INSERT INTO `trinity_string` VALUES ('764', 'Wintergrasp is now under the control of the %s.', '', '', '', '', '', '', '', '');
INSERT INTO `trinity_string` VALUES ('765', 'Wintergrasp timer set to %s.', '', '', '', '', '', '', '', '');
INSERT INTO `trinity_string` VALUES ('766', 'Wintergrasp battle started.', '', '', '', '', '', '', '', '');
INSERT INTO `trinity_string` VALUES ('767', 'Wintergrasp battle finished.', '', '', '', '', '', '', '', '');
INSERT INTO `trinity_string` VALUES ('768', 'Wintergrasp info: %s controlled. Timer: %s. Wartime: %s. Number of Players: (Horde: %u, Alliance: %u)', '', '', '', '', '', '', '', '');
INSERT INTO `trinity_string` VALUES ('769', 'Wintergrasp outdoorPvP is disabled.', '', '', '', '', '', '', '', '');
INSERT INTO `trinity_string` VALUES ('770', 'Wintergrasp outdoorPvP is enabled.', '', '', '', '', '', '', '', '');
INSERT INTO `trinity_string` VALUES ('771', 'You have reached Rank 1: Corporal', '', '', '', '', '', '', '', 'Вы достигли Ранга 1: Капрал');
INSERT INTO `trinity_string` VALUES ('772', 'You have reached Rank 2: First Lieutenant', '', '', '', '', '', '', '', 'Вы достигли Ранга 2: Лейтенант');
REPLACE INTO `trinity_string` VALUES ('780', 'Before the Battle of  Wintergrasp left 30 minutes!', '', '', '', '', '', '', '', 'До битвы на  Озере Ледяных Оков осталось 30 минут!');
REPLACE INTO `trinity_string` VALUES ('781', 'Before the Battle of  Wintergrasp left 10 minutes!', '', '', '', '', '', '', '', 'До битвы на  Озере Ледяных Оков осталось 10 минут!');
REPLACE INTO `trinity_string` VALUES ('782', 'The battle for Wintergrasp  has stopped! Not enough defenders. Wintergrasp Fortress remains  Attackers.', '', '', '', '', '', '', '', 'Битва за Озеро Ледяных Оков Остановлена. Не хватает защитников. Крепость переходит атакующей  стороне.');
REPLACE INTO `trinity_string` VALUES ('783', 'The battle for Wintergrasp  has stopped! Not enough attackers. Wintergrasp Fortress remains  Defenders.', '', '', '', '', '', '', '', 'Битва за Озеро Ледяных Оков Остановлена. Не хватает нападающих. Крепость остается защитникам.');

DELETE FROM `command` WHERE name IN ('wg','wg enable','wg start','wg status','wg stop','wg switch','wg timer');
INSERT INTO `command` VALUES ('wg', '3', 'Syntax: .wg $subcommand.');
INSERT INTO `command` VALUES ('wg enable', '3', 'Syntax: .wg enable [on/off] Enable/Disable Wintergrasp outdoorPvP.');
INSERT INTO `command` VALUES ('wg start', '3', 'Syntax: .wg start\r\nForce Wintergrasp battle start.');
INSERT INTO `command` VALUES ('wg status', '3', 'Syntax: .wg status\r\nWintergrasp info, defender, timer, wartime.');
INSERT INTO `command` VALUES ('wg stop', '3', 'Syntax: .wg stop\r\nForce Wintergrasp battle stop (No rewards).');
INSERT INTO `command` VALUES ('wg switch', '3', 'Syntax: .wg switch\r\nSwitchs Wintergrasp defender team.');
INSERT INTO `command` VALUES ('wg timer', '3', 'Syntax: .wg timer $minutes\r\nChange the current timer. Min value = 1, Max value 60 (Wartime), 1440 (Not Wartime)');

/* Temp removed gameobject stopping you getting to the relic
* 60070 - [Wintergrasp Keep Collision Wall X:5396.209961 Y:2840.010010 Z:432.268005 MapId:571
* 60476 - [Doodad_WG_Keep_Door01_collision01 X:5397.109863 Y:2841.540039 Z:425.901001 MapId:571]*/
DELETE FROM gameobject WHERE id IN ('194323', '194162');

/*Defender's Portal Activate Proper Spell*/
REPLACE INTO `spell_linked_spell` VALUES ('54640','54643','0','Defender\'s Portal Activate Proper Spell');

/* Teleport WG SPELLs*/
REPLACE INTO `spell_target_position` VALUES ('59096', '571', '4561.58', '2835.33', '389.79', '0.34');
REPLACE INTO `spell_target_position` VALUES ('60035', '571', '5325.06', '2843.36', '409.285', '3.20278');

DELETE FROM `outdoorpvp_template` WHERE `TypeId` IN (7);
INSERT INTO `outdoorpvp_template` (`TypeId`, `ScriptName`, `Comment`) VALUES (7, 'outdoorpvp_wg', 'Wintergrasp');