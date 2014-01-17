update version_number set version_number=164, min_version_number=164;

ALTER TABLE "MARKET_AUCTIONS" RENAME COLUMN "USER_DESCRIPTION" TO "USER_DESCRIPTION_OLD";
ALTER TABLE "MARKET_AUCTIONS" RENAME COLUMN "OOB" TO "OOB_OLD";
ALTER TABLE "MARKET_AUCTIONS" RENAME COLUMN "ITEM_NAME" TO "ITEM_NAME_OLD";

ALTER TABLE "MARKET_AUCTIONS"
ADD
    (
      "USER_DESCRIPTION" VARCHAR2(4000) NULL,
      "OOB"   VARCHAR2(4000) NULL,
      "ITEM_NAME" VARCHAR2(4000) NULL
    ) ;

declare
cnt integer;

begin
select count(*) into cnt from user_indexes
where index_name = 'LOCATION_IDX';

if (cnt = 0) then
	execute immediate 'CREATE INDEX LOCATION_IDX ON MARKET_AUCTIONS( LOCATION ) TABLESPACE INDEXES';
end if;
      
exception
when others then
NULL;      

end;
/

