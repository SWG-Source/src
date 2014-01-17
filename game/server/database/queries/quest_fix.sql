create or replace package quest_fix as
	function remove_several_quests(quests varchar2, targetLength number) return varchar2;
	function remove_quest(quests varchar2) return varchar2;
end;
/

create or replace package body quest_fix as

function remove_quest(quests varchar2) return varchar2 as
	firstColon number;
	secondColon number;
	possibleNumber number;
	foundNumber number:=0;
begin
	firstColon := instr(quests,':');
	secondColon := instr(quests,':',firstColon+1);
	while (foundNumber = 0)
	loop
		begin
			possibleNumber := to_number(substr(quests,firstColon+1, secondColon-firstColon-1));
			foundNumber := 1;
		exception when others then
			firstColon := secondColon;
			secondColon := instr(quests,':',firstColon+1);
		end;
	end loop;

	return substr(quests,0,firstColon) || substr(quests,secondColon+1);
end;

function remove_several_quests(quests varchar2, targetLength number) return varchar2 as
	localQuests varchar2(5000);
begin
	localQuests:=quests;
	while (length(localQuests) > targetLength)
	loop
		localQuests := remove_quest(localQuests);
	end loop;
	return localQuests;
end;

end;
/

create table player_objects_save
as select * from player_objects;

update player_objects
set quests=quest_fix.remove_several_quests(quests,3500)
where length(quests) > 3500;

