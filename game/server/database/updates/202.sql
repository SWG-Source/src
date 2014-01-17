declare
  cnt number;
begin
  select count(*) into cnt
  from user_tables
  where table_name = 'ERRORLOG_LEVEL';
  if (cnt = 0) then
    execute immediate 'CREATE TABLE errorlog_level(levelid NUMBER(10) DEFAULT 0 NOT NULL)';
    execute immediate 'grant select on errorlog_level to public'; 
    execute immediate 'insert into errorlog_level (levelid) values (2)';
  end if;	
end;
/

declare
  cnt number;
begin
  select count(*) into cnt
  from user_tables
  where table_name = 'ERRORLOG_LEVEL_DESC';
  if (cnt = 0) then
    execute immediate 'CREATE TABLE ERRORLOG_LEVEL_DESC("LEVELID" NUMBER(10) DEFAULT 0 NOT NULL, "DESCRIP" VARCHAR2(50) NOT NULL)';
    execute immediate 'grant select on errorlog_level_desc to public'; 
  end if;	
end;
/

declare
  cnt number;
begin
  select count(*) into cnt
  from user_tables
  where table_name = 'ERRORLOG_LEVEL_DESC';
    if (cnt = 0) then
	insert into errorlog_level_desc (levelid,descrip) values (0,'DB Error logging disabled.');
	insert into errorlog_level_desc (levelid,descrip) values (1,'Basic DB Error logging enabled.');
	insert into errorlog_level_desc (levelid,descrip) values (2,'Detailed DB Error logging enabled.');
  end if;	
end;
/

declare
  cnt number;
begin
  select count(*) into cnt
  from user_tables
  where table_name = 'ERRORLOG_RERAISE_LIMITS';
  if (cnt = 0) then
    execute immediate 'CREATE TABLE ERRORLOG_RERAISE_LIMITS (PACKAGE VARCHAR2(30) NOT NULL, PROCEDURE VARCHAR2(30) NOT NULL,ERRORLIMIT NUMBER(10) DEFAULT 0 NOT NULL, ERRORCOUNT NUMBER(10) DEFAULT 0 NOT NULL, UNLIMITED_FLAG NUMBER(10) DEFAULT 0 NOT NULL,CONSTRAINT PK_RERAISE_LIMITS PRIMARY KEY(PACKAGE, PROCEDURE))';
    execute immediate 'grant select on errorlog_reraise_limits to public';
  end if;	
end;
/

update version_number set version_number=202, min_version_number=202;
