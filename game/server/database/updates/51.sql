alter table tangible_objects add damage_taken int;
alter table tangible_objects add invulnerable char(1);
alter table tangible_objects add custom_appearance varchar2(2000);

update tangible_objects set damage_taken = 0, invulnerable = 'N';

update version_number set version_number=51, min_version_number=51;
