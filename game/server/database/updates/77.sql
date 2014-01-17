create table delete_reasons -- NO_IMPORT
(
	reason_code number,
	tag varchar2(100),
	description varchar2(1000),
	constraint pk_delete_reasons primary key (reason_code) using index tablespace indexes
);
grant select on auction_bids to public;

insert into delete_reasons values (0,'DR_NotDeleted','not deleted');	
insert into delete_reasons values (1,'DR_Unknown','unknown');
insert into delete_reasons values (2,'DR_Decay','deleted by decay');
insert into delete_reasons values (3,'DR_God','deleted by a god client or console command');
insert into delete_reasons values (4,'DR_Player','deleted by a player action');
insert into delete_reasons values (5,'DR_Script','deleted by a script');
insert into delete_reasons values (6,'DR_CharacterDeleted','deleted because the player deleted the character');

alter table waypoint_objects add color varchar2(50);

update version_number set version_number=77, min_version_number=77;
