CREATE OR REPLACE  PACKAGE BODY "LOGGING" as
  procedure LOGGER(v_log_type_id IN varchar2, v_should_commit IN INTEGER, v_msg_id IN NUMBER, v_msg_text IN varchar2)
  as
  begin
    insert into logger ( log_type_id, msg_id, msg_text ) values ( v_log_type_id, v_msg_id, v_msg_text );
    if v_should_commit = 1 then
      commit;
    end if;  
  end logger;
  
  procedure LOG_STATUS(v_msg_id IN NUMBER, v_msg_text IN varchar2)
  as
  begin
    LOGGING.LOGGER('STATUS', 1, v_msg_id, v_msg_text );
  end log_status;  
  
  procedure LOG_ERROR(v_msg_id IN NUMBER, v_msg_text IN varchar2)
  as
  begin
    LOGGING.LOGGER('ERROR', 1, v_msg_id, v_msg_text );
  end log_error; 
    
end logging;