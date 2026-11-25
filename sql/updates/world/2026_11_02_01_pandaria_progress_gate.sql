-- Pandaria progress gate: Passage Keeper (700021) + unlock quest 700021
-- Requires one of every T4-T6 BC armor token (54). Script: npc_pandaria_passage_keeper

SET @Entry := 700021;
SET @Quest := 700021;
SET @Name  := 'Passage Keeper';

DELETE FROM `quest_template_addon` WHERE `ID` = @Quest;
DELETE FROM `quest_template` WHERE `ID` = @Quest;
INSERT INTO `quest_template` (
  `ID`, `QuestType`, `QuestLevel`, `MinLevel`, `QuestPackageID`, `QuestSortID`, `QuestInfoID`,
  `SuggestedGroupNum`, `RewardNextQuest`, `RewardXPDifficulty`, `RewardMoney`, `RewardBonusMoney`,
  `Flags`, `LogTitle`, `LogDescription`, `QuestDescription`, `AreaDescription`, `QuestCompletionLog`,
  `VerifiedBuild`
) VALUES (
  @Quest, 2, 85, 1, 0, 0, 0,
  0, 0, 0, 0, 0,
  65536,
  'Passage to Pandaria',
  'Turn in one of every Burning Crusade T4, T5, and T6 armor token (54) to the Passage Keeper.',
  'The mists of Pandaria will not part until you prove your worth with the tokens of Outland''s fallen champions.',
  '',
  'Return to the Passage Keeper when you have gathered every token.',
  18414
);

INSERT INTO `quest_template_addon` (`ID`, `MaxLevel`, `AllowableClasses`, `PrevQuestID`, `NextQuestID`, `ExclusiveGroup`, `SpecialFlags`)
VALUES (@Quest, 0, 0, 0, 0, 0, 0);

DELETE FROM `creature` WHERE `id` = @Entry;
DELETE FROM `creature_template_model` WHERE `CreatureID` = @Entry;
DELETE FROM `creature_template` WHERE `entry` = @Entry;

INSERT INTO `creature_template` (
  `entry`, `difficulty_entry_1`, `difficulty_entry_2`, `difficulty_entry_3`, `difficulty_entry_4`, `difficulty_entry_5`,
  `KillCredit1`, `KillCredit2`, `name`, `femaleName`, `subname`, `IconName`, `gossip_menu_id`,
  `minlevel`, `maxlevel`, `exp`, `exp_unk`, `faction`, `npcflag`, `npcflag2`,
  `speed_walk`, `speed_run`, `scale`, `rank`, `mindmg`, `maxdmg`, `dmgschool`, `attackpower`, `dmg_multiplier`,
  `BaseAttackTime`, `RangeAttackTime`, `unit_class`, `unit_flags`, `unit_flags2`, `dynamicflags`,
  `family`, `trainer_type`, `trainer_class`, `trainer_race`, `minrangedmg`, `maxrangedmg`, `rangedattackpower`,
  `type`, `type_flags`, `type_flags2`, `lootid`, `pickpocketloot`, `skinloot`,
  `resistance1`, `resistance2`, `resistance3`, `resistance4`, `resistance5`, `resistance6`,
  `spell1`, `spell2`, `spell3`, `spell4`, `spell5`, `spell6`, `spell7`, `spell8`,
  `PetSpellDataId`, `VehicleId`, `mingold`, `maxgold`, `AIName`, `MovementType`, `HoverHeight`,
  `Health_mod`, `Mana_mod`, `Mana_mod_extra`, `Armor_mod`, `RacialLeader`,
  `questItem1`, `questItem2`, `questItem3`, `questItem4`, `questItem5`, `questItem6`,
  `movementId`, `RegenHealth`, `VignetteID`, `TrackingQuestID`, `mechanic_immune_mask`, `flags_extra`,
  `ScriptName`, `VerifiedBuild`
) VALUES (
  @Entry, 0, 0, 0, 0, 0,
  0, 0, @Name, '', 'Pandaria Progress Gate', NULL, 0,
  90, 90, 4, 0, 35, 1, 0,
  1, 1.14286, 1, 0, 0, 0, 0, 0, 1,
  2000, 0, 1, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0,
  7, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, '', 0, 1,
  1, 1, 1, 1, 0,
  0, 0, 0, 0, 0, 0,
  0, 1, 0, 0, 0, 2,
  'npc_pandaria_passage_keeper', 18414
);

INSERT INTO `creature_template_model`
(`CreatureID`, `Idx`, `CreatureDisplayID`, `DisplayScale`, `Probability`, `VerifiedBuild`)
VALUES
(@Entry, 0, 19646, 1, 1, 18414);

INSERT INTO `creature`
(`id`, `map`, `zoneId`, `areaId`, `spawnMask`, `phaseMask`, `modelid`, `equipment_id`,
 `position_x`, `position_y`, `position_z`, `orientation`,
 `spawntimesecs`, `wander_distance`, `currentwaypoint`, `curhealth`, `curmana`, `MovementType`)
VALUES
(@Entry, 530, 3703, 3703, 1, 1, 0, 0,
 -1838.16, 5301.79, -12.428, 1.57,
 300, 0, 0, 1, 0, 0);

INSERT INTO `creature`
(`id`, `map`, `zoneId`, `areaId`, `spawnMask`, `phaseMask`, `modelid`, `equipment_id`,
 `position_x`, `position_y`, `position_z`, `orientation`,
 `spawntimesecs`, `wander_distance`, `currentwaypoint`, `curhealth`, `curmana`, `MovementType`)
VALUES
(@Entry, 0, 1519, 5148, 1, 1, 0, 0,
 -8867.5, 628.4, 96.2, 3.9,
 300, 0, 0, 1, 0, 0);

INSERT INTO `creature`
(`id`, `map`, `zoneId`, `areaId`, `spawnMask`, `phaseMask`, `modelid`, `equipment_id`,
 `position_x`, `position_y`, `position_z`, `orientation`,
 `spawntimesecs`, `wander_distance`, `currentwaypoint`, `curhealth`, `curmana`, `MovementType`)
VALUES
(@Entry, 1, 1637, 5170, 1, 1, 0, 0,
 1632.0, -4439.0, 15.4, 3.6,
 300, 0, 0, 1, 0, 0);
