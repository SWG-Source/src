declare
	type curtype is ref cursor;
	index_list curtype;
	index_name varchar2(200);
begin
	dbms_output.enable(20000);

	open index_list for
	select index_name from user_indexes where table_name = 'OBJECT_VARIABLES' and index_name not like 'SYS%';
	loop
		fetch index_list into index_name;
		exit when index_list%notfound;
		dbms_output.put_line('drop index ' || index_name);
		execute immediate 'drop index ' || index_name;
	end loop;
	close index_list;
end;
/

create index idx_object_variables_1 on object_variables(object_id) tablespace indexes;

update version_number set version_number=72, min_version_number=71;
