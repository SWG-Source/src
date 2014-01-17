set verify off
col object_id format 999999999999
col contained_by format 999999999999
col load_with format 999999999999
col location_id format 999999999999
col owner_id format 999999999999
col character_object format 999999999999
col item_id format 999999999999
select object_id, contained_by, load_with, deleted, deleted_date
from objects where object_id in (
select object_id from missing_objects where query_time > trunc(sysdate));

select item_id, location_id from market_auctions where item_id in (
select object_id from missing_objects where query_time > trunc(sysdate));