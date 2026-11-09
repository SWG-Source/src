-- 274: no schema change. Applying an update makes database_update.pl
-- reload the PL/SQL packages, which installs persister.set_clock keeping
-- the larger of the stored and the new last_save_time.
update version_number set version_number=274, min_version_number=274;
