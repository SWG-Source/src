on sqlerror exit failure rollback;

alter table creature_objects add (
	attribute_0 int,
	attribute_1 int,
	attribute_2 int,
	attribute_3 int,
	attribute_4 int,
	attribute_5 int,
	attribute_6 int,
	attribute_7 int,
	attribute_8 int,
	attribute_9 int,
	attribute_10 int,
	attribute_11 int,
	attribute_12 int,
	attribute_13 int,
	attribute_14 int,
	attribute_15 int,
	attribute_16 int,
	attribute_17 int,
	attribute_18 int,
	attribute_19 int,
	attribute_20 int,
	attribute_21 int,
	attribute_22 int,
	attribute_23 int,
	attribute_24 int,
	attribute_25 int,
	attribute_26 int);

update creature_objects
set 
	attribute_0 = (select value from attributes a where a.object_id = creature_objects.object_id and a.type=0),
	attribute_1 = (select value from attributes a where a.object_id = creature_objects.object_id and a.type=1),
	attribute_2 = (select value from attributes a where a.object_id = creature_objects.object_id and a.type=2),
	attribute_3 = (select value from attributes a where a.object_id = creature_objects.object_id and a.type=3),
	attribute_4 = (select value from attributes a where a.object_id = creature_objects.object_id and a.type=4),
	attribute_5 = (select value from attributes a where a.object_id = creature_objects.object_id and a.type=5),
	attribute_6 = (select value from attributes a where a.object_id = creature_objects.object_id and a.type=6),
	attribute_7 = (select value from attributes a where a.object_id = creature_objects.object_id and a.type=7),
	attribute_8 = (select value from attributes a where a.object_id = creature_objects.object_id and a.type=8),
	attribute_9 = (select value from attributes a where a.object_id = creature_objects.object_id and a.type=9),
	attribute_10 = (select value from attributes a where a.object_id = creature_objects.object_id and a.type=10),
	attribute_11 = (select value from attributes a where a.object_id = creature_objects.object_id and a.type=11),
	attribute_12 = (select value from attributes a where a.object_id = creature_objects.object_id and a.type=12),
	attribute_13 = (select value from attributes a where a.object_id = creature_objects.object_id and a.type=13),
	attribute_14 = (select value from attributes a where a.object_id = creature_objects.object_id and a.type=14),
	attribute_15 = (select value from attributes a where a.object_id = creature_objects.object_id and a.type=15),
	attribute_16 = (select value from attributes a where a.object_id = creature_objects.object_id and a.type=16),
	attribute_17 = (select value from attributes a where a.object_id = creature_objects.object_id and a.type=17),
	attribute_18 = (select value from attributes a where a.object_id = creature_objects.object_id and a.type=18),
	attribute_19 = (select value from attributes a where a.object_id = creature_objects.object_id and a.type=19),
	attribute_20 = (select value from attributes a where a.object_id = creature_objects.object_id and a.type=20),
	attribute_21 = (select value from attributes a where a.object_id = creature_objects.object_id and a.type=21),
	attribute_22 = (select value from attributes a where a.object_id = creature_objects.object_id and a.type=22),
	attribute_23 = (select value from attributes a where a.object_id = creature_objects.object_id and a.type=23),
	attribute_24 = (select value from attributes a where a.object_id = creature_objects.object_id and a.type=24),
	attribute_25 = (select value from attributes a where a.object_id = creature_objects.object_id and a.type=25),
	attribute_26 = (select value from attributes a where a.object_id = creature_objects.object_id and a.type=26);

drop table attributes;

update version_number set version_number=146, min_version_number=146;
