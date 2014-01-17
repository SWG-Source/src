create table player_quest_objects
(
	object_id number(20),  -- BIND_AS(DB::BindableNetworkId)
	title varchar2(256),
	description varchar2(4000),
	creator number(20), -- BIND_AS(DB::BindableNetworkId)
	total_tasks int,
	difficulty int,
	task_title1 varchar2(128),
	task_description1 varchar2(4000),
	task_title2 varchar2(128),
	task_description2 varchar2(4000),
	task_title3 varchar2(128),
	task_description3 varchar2(4000),
	task_title4 varchar2(128),
	task_description4 varchar2(4000),
	task_title5 varchar2(128),
	task_description5 varchar2(4000),
	task_title6 varchar2(128),
	task_description6 varchar2(4000),
	task_title7 varchar2(128),
	task_description7 varchar2(4000),
	task_title8 varchar2(128),
	task_description8 varchar2(4000),
	task_title9 varchar2(128),
	task_description9 varchar2(4000),
	task_title10 varchar2(128),
	task_description10 varchar2(4000),
	task_title11 varchar2(128),
	task_description11 varchar2(4000),
	task_title12 varchar2(128),
	task_description12 varchar2(4000),
	constraint pk_player_quest_objects primary key (object_id) 
);
grant select on player_quest_objects to public;

update version_number set version_number=265, min_version_number=265;