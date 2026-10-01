-- A new database's accounts are keyed by the gcc32 station id rule
-- (see StationIdFromAccountName.h).
insert into station_id_scheme (id, scheme, basis) values (1, 'gcc32', 'new');
commit;
