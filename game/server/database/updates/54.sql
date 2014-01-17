create table loadbeacon_server_map
(
	object_template varchar2(255),
	server_id int,
	secondary_server_id int,
	constraint pk_loadbeacon_server_map primary key (object_template) using index tablespace indexes	
);

grant select on loadbeacon_server_map to public;

insert into loadbeacon_server_map (object_template,server_id,secondary_server_id)
	values ('object/tangible/loadbeacon/loadbeacon_large.iff',1,0);
insert into loadbeacon_server_map (object_template,server_id,secondary_server_id)
	values ('object/tangible/loadbeacon/loadbeacon_medium.iff',1,0);
insert into loadbeacon_server_map (object_template,server_id,secondary_server_id)
	values ('object/tangible/loadbeacon/loadbeacon_server_a.iff',1,0);
insert into loadbeacon_server_map (object_template,server_id,secondary_server_id)
	values ('object/tangible/loadbeacon/loadbeacon_server_b.iff',2,1);
insert into loadbeacon_server_map (object_template,server_id,secondary_server_id)
	values ('object/tangible/loadbeacon/loadbeacon_server_c.iff',3,2);
insert into loadbeacon_server_map (object_template,server_id,secondary_server_id)
	values ('object/tangible/loadbeacon/loadbeacon_server_d.iff',4,3);
insert into loadbeacon_server_map (object_template,server_id,secondary_server_id)
	values ('object/tangible/loadbeacon/loadbeacon_server_e.iff',5,4);

update version_number set version_number=54, min_version_number=52;
