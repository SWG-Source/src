select character_object, uc_character_name, character_full_name
from players
where INSTR(LOWER(character_full_name), uc_character_name) = 0;

