alter table auction_locations add (empty_date number(20), last_access_date number(20), inactive_date number(20));
update auction_locations
set empty_date = 0,
    last_access_date = (sysdate - to_date('01/01/1970', 'MM/DD/YYYY') + 7/24) * 24 * 3600,
    inactive_date = 0;
alter table auction_locations modify (empty_date not null, last_access_date not null, inactive_date not null);
update version_number set version_number=171, min_version_number=171;
