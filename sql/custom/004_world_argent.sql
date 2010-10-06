UPDATE `creature_template` SET `ScriptName`='npc_training_dummy_argent' WHERE `entry` IN ('33272','33243','33229');
UPDATE creature_template SET ScriptName = 'npc_squire_david' WHERE entry = 33447;
UPDATE creature_template SET ScriptName = 'npc_argent_valiant' WHERE entry = 33448;
UPDATE creature_template SET ScriptName = 'npc_squire_danny' WHERE entry = 33518;
UPDATE creature_template SET ScriptName = 'npc_argent_champion' WHERE entry = 33707;

-- Quest : A Worthy Weapon
DELETE FROM `script_texts` WHERE `entry` IN (-1850000,-1850001,-1850002,-1850003);
INSERT INTO `script_texts` (`npc_entry`, `entry`, `content_default`, `content_loc2`,`comment`) VALUES
(0, -1850000, 'Oh, these are hyacinths \ winter? For me ?', 'Oh, these are hyacinths \ winter? For me  ?',''),
(0, -1850001, 'We don\'t had not brought flowers here for so long.', 'We don\'t had not brought flowers here for so long.',''),
(0, -1850002, 'The lake is a lonely spot some years. Travelers to come over, and evil has invaded the waters.', 'The lake is a lonely spot some years. Travelers to come over, and evil has invaded the waters.',''),
(0, -1850003, 'Your gift shows a rare kindness, traveler. Please, take this blade as a token of my gratitude. There has long, is another traveler who had left here, but I do not need. ',' Your gift reveals a rare kindness, traveler. Please, take this blade as a token of my gratitude. Long ago, another passenger who had left here, but I have no use.','');
DELETE FROM `event_scripts` WHERE `id`=20990;
INSERT INTO `event_scripts` (`id`, `delay`, `command`, `datalong`, `datalong2`, `x`, `y`, `z`, `o`) VALUES (20990, 0, 10, 33273, 42000, 4602.977, -1600.141, 156.7834, 0.7504916);
UPDATE `creature_template` SET `InhabitType`=5, `ScriptName`='npc_maiden_of_drak_mar' WHERE `entry`=33273;
DELETE FROM `creature_template_addon` WHERE `entry`=33273;
INSERT INTO `creature_template_addon` (`entry`, `emote`) VALUES (33273, 13); -- 13 = EMOTE_STATE_SIT


-- Text campioni
-- Quest : A Worthy Weapon
DELETE FROM `script_texts` WHERE `entry` IN (-1850000,-1850001,-1850002,-1850003);
INSERT INTO `script_texts` (`npc_entry`, `entry`, `content_default`, `content_loc2`,`comment`) VALUES
(0, -1850004, 'Stand ready !', 'Stand ready !',''),
(0, -1850005, 'Let the battle begins !', 'Let the battle begins !',''),
(0, -1850006, 'Prepare your self !', 'Prepare your self! !',''),
(0, -1850007, 'You think you have the courage in you? Will see.', 'You think you have the courage in you? Will see.',''),
(0, -1850008, 'Impressive demonstration. I think you\re quite able to join the ranks of the valiant.', 'Impressive demonstration. I think you\re quite able to join the ranks of the valiant.',''),
(0, -1850009, 'I\'ve won. You\'ll probably have more luck next time.', 'I\'ve won. You\'ll probably have more luck next time.',''),
(0, -1850010, 'I am defeated. Nice battle !', 'I am defeated. Nice battle !',''),
(0, -1850011, 'It seems that I\'ve underestimated your skills. Well done.', 'It seems that I\'ve underestimated your skills. Well done.',''),
(0, -1850012, 'You\'ll probably have more luck next time.', '','');

-- Vendors
UPDATE creature_template SET ScriptName = 'npc_vendor_argent_tournament' WHERE entry IN (33553, 33554, 33556,33555,33557,33307,33310,33653,33650,33657);