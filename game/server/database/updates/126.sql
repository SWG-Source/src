alter table tangible_objects drop (armor_effectiveness,decay_time,hit_points,is_frozen,is_squelched,popup_help_id,quality_rating,weight);
alter table tangible_objects add (creator_id number(20), source_draft_schematic int);

update version_number set version_number=126, min_version_number=126;
