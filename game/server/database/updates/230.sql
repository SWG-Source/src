declare
  cnt number;
begin
  select count(*) into cnt
  from user_tables
  where table_name = 'CHARACTER_PROFILE';
  if (cnt = 0) then
	execute immediate'CREATE TABLE character_profile(cluster_name VARCHAR2 (50) NOT NULL, character_id NUMBER(20) NOT NULL, character_name  VARCHAR2 (127), object_name  VARCHAR2 (127), x  FLOAT, y FLOAT, z FLOAT, scene_id VARCHAR2 (50), cash_balance INTEGER, bank_balance      INTEGER, object_template_name VARCHAR2 (500), station_id NUMBER(20), contained_by NUMBER(20), create_time DATE, played_time              DATE , num_lots NUMBER(20), constraint pk_character_profile primary key (cluster_name, character_id) )';

	execute immediate 'grant select on character_profile to public';
  end if;
end;
/

update version_number set version_number=230, min_version_number=230;