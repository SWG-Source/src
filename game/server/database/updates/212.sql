declare
  cnt number;
begin
  select count(*) into cnt
  from user_tables
  where table_name = 'MARKET_AUCTION_ATTRIBUTES';
  if (cnt = 0) then
     execute immediate 'CREATE TABLE MARKET_AUCTION_ATTRIBUTES ( item_id number(20), attribute_name varchar2(1000), attribute_value varchar2(1000), constraint pk_market_auction_attributes primary key (item_id, attribute_name) )';
  end if;
end;
/

declare
  cnt number;
begin
  select count(*) into cnt
  from user_sequences
  where sequence_name = 'SEQ_QUERIES';
  if (cnt = 0) then
    execute immediate 'CREATE SEQUENCE SEQ_QUERIES INCREMENT BY 1 START WITH 12 MAXVALUE 1.0E28 MINVALUE 1 NOCYCLE CACHE 20 NOORDER';
  end if;
end;
/

declare
  cnt number;
begin
  select count(*) into cnt
  from user_tables
  where table_name = 'AUTOMATED_QUERIES';
  if (cnt = 0) then
     execute immediate 'CREATE TABLE AUTOMATED_QUERIES ( ID NUMBER(10) NOT NULL, STARTTIME DATE, ENDTIME DATE, CREATETIME DATE DEFAULT sysdate NOT NULL, STATUS VARCHAR2(30) DEFAULT ''ready'' NOT NULL, TYPE VARCHAR2(30) NOT NULL, RUN_SERIAL NUMBER(10) DEFAULT 0 NOT NULL, EXEC_ORDER NUMBER(10) DEFAULT 0, QUERY_TEXT VARCHAR2(4000) NOT NULL)';

  end if;
end;
/

update version_number set version_number=212, min_version_number=212;
