alter table auction_locations add search_enabled char(1) default 'Y';
update auction_locations set search_enabled = 'N'
where owner_id <> 0;
