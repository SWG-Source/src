/*
	SQL for gold database update 20090829
*/

--------------------------------------------------------------------------------

/*
	Remove corellia-style wall (object/static/structure/corellia/corl_imprv_wall_4x16_s01.iff)
	Note: world snapshot update required with this change
*/

update objects
set deleted = 8, -- deleted by the publish process
    deleted_date = sysdate
where object_id = 2195443
  and deleted = 0;

commit;
