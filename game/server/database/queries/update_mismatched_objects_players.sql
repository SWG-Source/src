begin

declare
o_id objects.object_id%type;
f_name players.character_full_name%type;

cursor n_list is
select objects.object_id, objects.object_name
from objects, players 
where objects.object_id = players.character_object
and objects.object_name != players.CHARACTER_FULL_NAME;

begin
open n_list;

loop
fetch n_list into o_id, f_name;
exit when n_list%notfound;

   update players set players.CHARACTER_FULL_NAME = f_name where players.character_object = o_id;

end loop;

close n_list;
end;
end;


