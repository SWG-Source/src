create or replace type vaoflongstring is varray(10000) of varchar2(4000);
/

update version_number set version_number=165, min_version_number=165;
