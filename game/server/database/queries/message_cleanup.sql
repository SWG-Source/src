create table messages_new tablespace &tablespace
as select m.*
from messages m, objects o
where m.target = o.object_id;

drop table messages;

rename messages_new to messages;

alter table messages add constraint pk_messages primary key (message_id) using index tablespace &index_tablespace;

create index object_idx on messages (target) tablespace &index_tablespace;

analyze table messages compute statistics;
