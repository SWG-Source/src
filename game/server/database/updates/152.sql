update version_number set version_number=152, min_version_number=152;

alter table swg_characters add("ENABLED" CHAR(1) DEFAULT 'Y' NOT NULL);

