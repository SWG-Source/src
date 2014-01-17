create table publish_player_fixup --NO_IMPORT
(
	player_object_id number(20),
	x number,
	y number,
	z number,
	constraint pk_publish_player_fixup primary key (player_object_id) using index tablespace indexes
);
grant select on publish_player_fixup to public;

create table scruncher -- NO_IMPORT
(
	new_object_id number(20),
	old_object_id number(20),
	constraint pk_scruncher primary key (old_object_id) using index tablespace indexes
);
grant select on scruncher to public;

create global temporary table temp_free_object_ids
(
	start_id int,
	end_id int
);
grant select on temp_free_object_ids to public;

alter table free_object_ids drop column game_server_id;

update version_number set version_number=71, min_version_number=71;
