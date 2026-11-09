-- 274: no schema change. Applying an update makes database_update.pl
-- reload the PL/SQL packages, which installs persister.set_clock keeping
-- the larger of the stored and the new last_save_time.
-- min_version_number is not raised: servers expecting 273 work unchanged
-- with the new set_clock, and raising it would stop them from starting.
update version_number set version_number=274;
