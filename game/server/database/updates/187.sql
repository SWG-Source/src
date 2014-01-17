alter table players add last_login_time date default sysdate;
update players set last_login_time = to_date('06/01/03', 'mm/dd/yy');
update version_number set version_number=187, min_version_number=187;

