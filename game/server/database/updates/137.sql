
alter table property_lists drop constraint pk_property_lists;
alter table property_lists drop column sequence_no;
delete from property_lists a where a.rowid > any (select b.rowid from property_lists b where a.object_id=b.object_id and a.list_id=b.list_id and a.value=b.value);
delete from property_lists where object_id is null or list_id is null or value is null;
alter table property_lists add constraint pk_property_lists primary key (object_id, list_id, value) using index tablespace indexes;

update version_number set version_number=137, min_version_number=137;

