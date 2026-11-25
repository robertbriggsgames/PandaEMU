-- One-shot: grandfather characters already at level 90 when Pandaria gate goes live.
-- Do not re-run blindly later (would unlock new 90s without tokens).
INSERT IGNORE INTO `character_queststatus_rewarded` (`guid`, `quest`, `active`)
SELECT `guid`, 700021, 1 FROM `characters` WHERE `level` >= 90;
