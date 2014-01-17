alter table accounts drop column num_lots;

update version_number set version_number=154, min_version_number=154;
