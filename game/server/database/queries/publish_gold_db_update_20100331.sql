/*
	SQL for gold database update 20100331
*/

--------------------------------------------------------------------------------

/*
	Remove the signs for Update 16 - Bug Bash.

	Note: world snapshot update required with this change

	The following templates are removed:

		          ID NAME
		------------ -------------------------------------------
		   182134188 object/tangible/sign/all_sign_city_s03.iff
		  1138115105 object/tangible/sign/all_sign_city_s02.iff
		 -1732121930 object/tangible/sign/all_sign_city_s01.iff
*/

update objects
set deleted = 8, -- deleted by the publish process
    deleted_date = sysdate,
	load_with = null
where object_template_id in (
		-- object/tangible/sign/all_sign_city_s03.iff
		182134188,
		-- object/tangible/sign/all_sign_city_s02.iff
		1138115105,
		-- object/tangible/sign/all_sign_city_s01.iff
		-1732121930
	) and deleted = 0;

/*
	Fix condition on persisted spawners to stop presents from float over them.

	Note: *NO* world snapshot update required with this change

	These spawners have the wrong condition and are being set to 256 (CONDITION_INVULNERABLE)

   OBJECT_ID NAME                                                CONDITION SCENE_ID                    X            Y            Z
------------ ------------------------------------------------ ------------ ---------------- ------------ ------------ ------------
     4235520 object/tangible/region/deliver_region_10.iff            -9191 tatooine         -1033.882813           12  -3644.68042
     4235522 object/tangible/region/deliver_region_10.iff            -9191 tatooine         -1226.865356           12 -3628.592529
     4235525 object/tangible/region/deliver_region_10.iff            -9191 tatooine         -1429.003296           12 -3628.025146
     4235530 object/tangible/region/deliver_region_10.iff            -9191 tatooine         -1429.785889           10 -3773.310547
     4235532 object/tangible/region/deliver_region_10.iff            -9191 tatooine         134.86875916           52 -5333.142578
     4235533 object/tangible/region/deliver_region_10.iff            -9191 tatooine         114.68991852           52   -5404.1875
     4235535 object/tangible/region/deliver_region_10.iff            -9191 tatooine         123.40797424           52 -5377.308105
     4235541 object/tangible/region/deliver_region_10.iff            -9191 tatooine         -5128.779297           75 -6542.421387
     4235556 object/tangible/region/deliver_region_10.iff              768 tatooine         3951.1103516 54.446979523 2282.9345703
     4235561 object/tangible/region/deliver_region_10.iff            -9191 tatooine          3364.032959            5 -4830.193359
     4275354 object/tangible/region/deliver_region_10.iff            -9191 tatooine         -2874.249756            5 2546.2949219
*/

update tangible_objects set condition = 256 where object_id in
(
	4235520,
	4235522,
	4235525,
	4235530,
	4235532,
	4235533,
	4235535,
	4235541,
	4235556,
	4235561,
	4275354
);

commit;
