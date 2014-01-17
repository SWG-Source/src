create or replace type vaofnumber is varray(10000) of number;
/

create or replace type vaofstring is varray(10000) of varchar2(1000);
/

update version_number set version_number=133, min_version_number=133;
