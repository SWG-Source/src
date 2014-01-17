update biographies
set biography = 'Too large ... truncated.'
where lengthb(biography) > 1024;
 
alter table biographies modify(biography varchar2(1024));

update version_number set version_number=207, min_version_number=207;

