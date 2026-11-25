-- While You're At It (24930): PrevQuestID was -14157 (must have Old Divisions ACTIVE).
-- That window is easy to miss; ExclusiveGroup -24930 then soft-locks The Rebel Lord's Arsenal (14159)
-- until both 24930 and Brothers In Arms (26129) are rewarded. Zero characters had ever taken 24930.
-- Change to +14157 so completing Old Divisions unlocks the side quest from Lord Godfrey (35115).

UPDATE `quest_template_addon`
SET `PrevQuestID` = 14157
WHERE `ID` = 24930 AND `PrevQuestID` = -14157;
