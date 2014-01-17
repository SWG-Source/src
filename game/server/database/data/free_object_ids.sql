delete from free_object_ids;

insert into free_object_ids (start_id)
select nvl(max(object_id)+1,10000001)
from objects;
