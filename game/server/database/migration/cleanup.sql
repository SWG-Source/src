-- Takes an Oracle database prepared by the makefile
-- and sets it up to recieve data from a PostgreSQL
-- migration

delete from free_object_ids;
delete from clock;
