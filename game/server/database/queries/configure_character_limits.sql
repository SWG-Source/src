set verify off
update default_char_limits set account_limit = &max_characters_per_account, cluster_limit = &max_characters_per_cluster;
update default_character_slots set num_slots = &character_slots where character_type_id = 1;
exit;
