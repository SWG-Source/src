alter table objects add cash_balance int;
alter table objects add bank_balance int;

update objects set cash_balance = nvl((select value from object_variables ov where objects.object_id = ov.object_id and name = 'money.cash'),0);
update objects set bank_balance = nvl((select value from object_variables ov where objects.object_id = ov.object_id and name = 'money.bank'),0);
delete from object_variables where name='money.cash' or name = 'money.bank';

create table named_bank_accounts  -- NO_IMPORT
(
	account_id number(20),
	account_name varchar2(80),
	constraint pk_named_bank_accounts primary key (account_id) using index tablespace indexes
);

create table bank_record  -- NO_IMPORT
(
	transaction_id number(20),
	transaction_type number,
	source number(20),
	target number(20),
	amount number,
	constraint pk_bank_record primary key (transaction_id) using index tablespace indexes
);

update version_number set version_number=47, min_version_number=47;
