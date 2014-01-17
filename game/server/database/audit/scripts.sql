create table script_log
(
	UPDATE_TIME DATE,
	OBJECT_ID NUMBER,
	SEQUENCE_NO NUMBER,
	SCRIPT VARCHAR2(100),
	UPDATE_TYPE CHAR(1)	
);

create or replace trigger audit_script_insert
before insert on scripts
for each row
	begin
		insert into script_log values(sysdate, :new.object_id, :new.sequence_no, :new.script,'A');
	end;
/

create or replace trigger audit_script_delete
before delete on scripts
for each row
	begin
		insert into script_log values(sysdate, :old.object_id, :old.sequence_no, :old.script,'D');
	end;
/
