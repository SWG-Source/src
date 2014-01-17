-- This script prints out truncate and copy commands
-- Use it to build the copy_from_database script.

set pagesize 0
set line 500
set escape \

select 'truncate table ' || table_name || ';' from user_tables order by table_name;

select 'insert into ' || table_name || ' select * from \&\&schema..' || table_name || ';' from user_tables order by table_name;
