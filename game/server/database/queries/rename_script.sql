whenever sqlerror exit failure rollback
set verify off

accept old_script_name char prompt 'Enter the old script name:  ';
accept new_script_name char prompt 'Enter the new script name:  ';

update scripts
set script='&&new_script_name'
where script='&&old_script_name';

UNDEFINE new_script_name
UNDEFINE old_script_name
set verify on
