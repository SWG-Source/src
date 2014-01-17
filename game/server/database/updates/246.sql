update market_auction_attributes set ATTRIBUTE_VALUE=CONCAT(ATTRIBUTE_VALUE,'0') where ATTRIBUTE_NAME = 'resource_contents' and ATTRIBUTE_VALUE like '%/100000';
update version_number set version_number=246, min_version_number=246;
