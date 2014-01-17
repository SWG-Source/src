select character_full_name, uc_character_name
from players
where uc_character_name != SUBSTR(LOWER(character_full_name), 1, INSTR(character_full_name || ' ', ' ', 1)-1)

