/*
	SQL for gold database update 20080929
*/

--------------------------------------------------------------------------------

/*
	Remove giant spire
	Note: world snapshot update required with this change
*/

update objects
set deleted = 8 -- deleted by the publish process
where object_id = 60367;

/*
	Move spawner
	Note: no world snapshot update required
*/

update objects
set x = -2514, y = 130, z = 1481
where object_id = 4005699;

commit;


