select objects.object_id, objects.object_name
from objects, players 
where objects.object_id = players.character_object
and objects.object_name != players.CHARACTER_FULL_NAME;
