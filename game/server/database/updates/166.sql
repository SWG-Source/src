update version_number set version_number=166, min_version_number=166;

declare
cnt integer;

begin

select count(*) into cnt from user_indexes
where index_name = 'CHUNK_IDX';

if (cnt > 0) then
	execute immediate 'DROP INDEX CHUNK_IDX';
end if;
      
select count(*) into cnt from user_indexes
where index_name = 'OBJECTS_NODE_CONTAINED_BY_IDX';

if (cnt = 0) then
	execute immediate 'CREATE INDEX OBJECTS_NODE_CONTAINED_BY_IDX ON OBJECTS(NODE_X, NODE_Z, CONTAINED_BY, DELETED) tablespace indexes';
end if;

exception
when others then
NULL;      

end;
/