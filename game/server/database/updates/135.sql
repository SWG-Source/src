alter table auction_locations add sales_tax number(20);
alter table auction_locations add sales_tax_bank_id number(20);
update auction_locations set sales_tax = 0;
update auction_locations set sales_tax_bank_id = 0;

update version_number set version_number=135, min_version_number=135;
