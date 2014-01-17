delete object_variable_names
where id not in (select name_id from  object_variables);
commit;

create table orphan_objects(object_id number, contained_by number, deleted number, player_controlled char(1));
alter table orphan_objects add constraint orphan_objects_pk primary key (object_id);
insert into orphan_objects(object_id, contained_by, deleted, player_controlled)
select object_id, contained_by, deleted, player_controlled
from objects;
commit;

set serveroutput on
declare
  cnt number := 0;
  total number := 0;
  deleted_code number;
  rowcount number := 0;
  cursor c1 is select object_id, contained_by from orphan_objects
  where contained_by > 10000000 and deleted = 0 and player_controlled = 'N';
begin
  loop
    cnt := 0;
    rowcount := 0;
    for c1rec in c1 loop
      begin
        deleted_code := 0;
        select deleted into deleted_code
        from orphan_objects
        where object_id = c1rec.contained_by;
        if (deleted_code <> 0) then
          update orphan_objects
          set deleted = 1
          where object_id = c1rec.object_id;
          rowcount := rowcount + 1;
          cnt := cnt + 1;
        end if;
      exception
        when no_data_found then
          update orphan_objects
	  set deleted = 1
          where object_id = c1rec.object_id;
          rowcount := rowcount + 1;
          cnt := cnt + 1;
      end;
      if (rowcount >= 1000) then
        rowcount := 0;
        commit;
      end if;
    end loop;
    commit;
    dbms_output.put_line('Found ' || cnt || ' orphan objects');
    total := total + cnt;
    exit when cnt = 0;
  end loop;
  dbms_output.put_line('Total orphan found: ' || total);
end;
/

update objects o
set deleted = 1, deleted_date = sysdate, load_with = null
where deleted = 0 and
exists (select 1 from orphan_objects oo where oo.object_id = o.object_id and oo.deleted = 1);
commit;

drop table orphan_objects;


