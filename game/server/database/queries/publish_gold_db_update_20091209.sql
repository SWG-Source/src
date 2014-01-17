/*
	SQL for gold database update 20091209
*/

--------------------------------------------------------------------------------

/*
	Remove the city structures for update 15 - GCW.

	Note: world snapshot update required with this change
*/

update objects
set deleted = 8, -- deleted by the publish process
    deleted_date = sysdate,
	load_with = null
where object_id in (
		-- arches and gates (Dearic, Talus)
		3365400,
		3335494,
		3335493,
		3335489,
		3305409,
		3305405,
		-- imperial banners (Bestine, Tatooine)
		1254613,
		1254614,
		9745402,
		9745412,
		1281518,
		1281520,
		-- arc's that vehicles pass through
		1177942,
		1177952,
		1177953,
		1177517,
		1177628,
		1177875,
		1177708, -- visual in psd 
		1177715, -- visual in psd 
		6714207,
		6714208,
		1178032,
		6714209,
		1281213,
		-- archways and bridges (Bestine, Tatooine)
		1281512,
		1177801,
		1177951,
		1177970,
		1177876
	) and deleted = 0;

commit;
