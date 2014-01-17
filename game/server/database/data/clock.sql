insert into clock (last_save_time)
select 0
from dual
where not exists (select * from clock);
