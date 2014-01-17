set serveroutput on

create table temp_owners (owner_id number primary key);

insert into temp_owners values (396029466);		
insert into temp_owners values (609420977);		
insert into temp_owners values (583009586);		
insert into temp_owners values (712020214);		
insert into temp_owners values (859026623);		
insert into temp_owners values (4716030246);		
insert into temp_owners values (2178866247);		
insert into temp_owners values (603410484);		
insert into temp_owners values (614493589);		
insert into temp_owners values (396029466);		
insert into temp_owners values (1063180785);		
insert into temp_owners values (4009484407);		
insert into temp_owners values (580027873);		
insert into temp_owners values (628312863);		
insert into temp_owners values (4245509948);		
insert into temp_owners values (503000949);
insert into temp_owners values (608001478);
insert into temp_owners values (587009481);
insert into temp_owners values (620909759);
insert into temp_owners values (5641741629);
insert into temp_owners values (1897114493);
insert into temp_owners values (581112766);
insert into temp_owners values (610345725);
insert into temp_owners values (4561337485);
insert into temp_owners values (764068362);
insert into temp_owners values (587009481);
insert into temp_owners values (727631341);
insert into temp_owners values (608471539);
insert into temp_owners values (248066507);
insert into temp_owners values (374018237);
insert into temp_owners values (601355504);
insert into temp_owners values (1081081666);
insert into temp_owners values (10835896373);
insert into temp_owners values (580019022);
insert into temp_owners values (322002961);
insert into temp_owners values (600176122);
insert into temp_owners values (606023191);
insert into temp_owners values (685536292);

analyze table temp_owners compute statistics;

begin
dbms_output.enable(1000);

for x in 
	(select o.object_id,o.deleted_date
	from building_objects b, tangible_objects t, objects o, temp_owners o
	where t.owner_id = o.owner_id
	and b.object_id = t.object_id
	and b.object_id = o.object_id
	and o.deleted <> 0)
loop

	update objects set bank_balance = bank_balance + 5000
	where object_id = x.object_id;
	
	for y in
		(select object_id
		from objects
		where deleted = 7 and abs(objects.deleted_date - x.deleted_date) < (10/1440)
		start with objects.object_id = x.object_id
		connect by prior object_id = contained_by
		and player_controlled = 'N')
	loop
		update objects
		set deleted_date = null, deleted = 0, load_with = x.object_id
		where object_id = y.object_id;

		dbms_output.put_line(y.object_id);
	end loop;		
end loop;
end;
/

drop table temp_owners;
