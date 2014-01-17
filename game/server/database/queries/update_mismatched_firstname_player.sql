UPDATE players SET uc_character_name = SUBSTR(LOWER(character_full_name), 1, INSTR(character_full_name || ' ', ' ', 1)-1)
WHERE character_object IN 
(SELECT character_object
FROM players
WHERE uc_character_name != SUBSTR(LOWER(character_full_name), 1, INSTR(character_full_name || ' ', ' ', 1)-1) )

