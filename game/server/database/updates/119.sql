-- Delete all accounts where there is another account with the same name and lower id number

delete named_bank_accounts
where exists (select 1 from named_bank_accounts n2
	where named_bank_accounts.account_name = n2.account_name
	and named_bank_accounts.account_id > n2.account_id);

create unique index account_name_idx on named_bank_accounts(account_name);  

update version_number set version_number=119, min_version_number=119;
