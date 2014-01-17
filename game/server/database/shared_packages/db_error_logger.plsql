create or replace package body db_error_logger
    as
  procedure dblogerror( p_ora_errno number, p_notes varchar2 default NULL)
  as
  
  PRAGMA AUTONOMOUS_TRANSACTION;
  
  v_ora_errmsg varchar2(255);
  v_schema varchar2(30);
  v_module varchar2(80);
  v_notes  varchar2(4000);
    
  begin
  
    v_ora_errmsg := SQLERRM(p_ora_errno);
    stack_search(2, v_schema, v_module);
    
    if length(p_notes) > 4000 then
    	v_notes := substr(p_notes,1,4000);
    else
    	v_notes := p_notes;	
    end if;
    	
    insert into errorlog 
    ( error_id, 
      ora_errno,
      ora_errmsg,
      schema_name,
      module_name,
      notes )
    values
    ( seq_errorlog.nextval,
      p_ora_errno,
      v_ora_errmsg,
      v_schema,
      v_module,
      v_notes );
  
      COMMIT;
      
  exception
  	when others then
		NULL;    
  end;
  
  procedure dblogerror_values( p_owner varchar2, p_name varchar2, p_type varchar2, p_value varchar2)
  as
  
  PRAGMA AUTONOMOUS_TRANSACTION;
  v_value  varchar2(4000);
  
  begin

    if length(p_value) > 4000 then
    	v_value := substr(p_value,1,4000);
    else
    	v_value := p_value;	
    end if;
  
    insert into errorlog_values
    ( error_id, 
      element_owner,
      element_name,
      element_type,
      element_value )
    values
    ( seq_errorlog.currval,
      p_owner,
      p_name,
      p_type,
      v_value );
  
      COMMIT;

  exception
  	when others then
		NULL;    
      
  end;
  
  procedure stack_search ( i_search in  integer, o_schema out varchar2, o_module out varchar2 )
  -----
  -- Returns the schema and module of the specified stack level
  -- (specified by i_search). Stack level 0 is this proc.
  -- Stack level 1 is this proc's caller. Level 2 is THEIR
  -- caller. And so on.
  -----
  as
    call_stack varchar2(4096) default dbms_utility.format_call_stack;
    n number;
    m_found_stack boolean default false;
    m_line varchar2(255);
    m_count number := 0;
            m_start integer := 1;
            m_end integer := -1;
  begin
            o_schema := 'Not found';
            o_module := 'Not found';
    loop
        m_end := instr( call_stack, chr(10), m_start );
        exit when ( m_count > i_search or m_end is NULL or m_end = 0 );
        m_line := ltrim(substr( call_stack, m_start + 20, m_end - (m_start + 20 )));
        m_start := m_end + 1;
                        -- Remove formatted stack header
        if ( NOT m_found_stack ) then
            if ( m_line like 'name%' ) then
                m_found_stack := TRUE;
            end if;
        else
            if ( m_count = i_search ) then
                if ( m_line like 'pr%' ) then
                    n := length( 'procedure ' );
                elsif ( m_line like 'fun%' ) then
                    n := length( 'function ' );
                elsif ( m_line like 'package body%' ) then
                    n := length( 'package body ' );
                elsif ( m_line like 'pack%' ) then
                    n := length( 'package ' );
                else
                    n := length( 'anonymous block ' );
                end if;
                m_line := substr( m_line, n );
                n := instr( m_line, '.' );
                o_schema := ltrim(rtrim(substr( m_line, 1, n-1 )));
                o_module := ltrim(rtrim(substr( m_line, n+1 )));
            end if;
            m_count := m_count + 1;
        end if;
    end loop;
end stack_search;
 
  function getloglevel 
  RETURN INTEGER
  as
  
  	m_level INTEGER;
	
  begin
  
  	select levelid
	into m_level
	from errorlog_level;
	
	return m_level;
	
  exception
  	when others then
	  return 0;
	  
  end getloglevel;
  	  

function reraisecheck ( p_package varchar2 default NULL, p_procedure varchar2 default NULL) 
return INTEGER
as

PRAGMA AUTONOMOUS_TRANSACTION;

	m_unlimited_errors INTEGER;
	m_errorlimit INTEGER;
	m_errorcount INTEGER;
begin

  if (p_package IS NULL or p_procedure IS NULL) then
  	return 1;
  else
  	update errorlog_reraise_limits
  	set errorcount = errorcount + 1
  	where package = p_package and
  	procedure = p_procedure;
  	
  	commit;
  	
  	if (sql%rowcount = 0)  then
  		return 1;
  	else
  		select unlimited_flag, errorlimit, errorcount
  		into m_unlimited_errors, m_errorlimit, m_errorcount
  		from errorlog_reraise_limits
  		where package = p_package and
  		procedure = p_procedure;
  		
  		if (m_unlimited_errors = 1) then
  			return 0;
  		else
  		  if (m_errorcount <= m_errorlimit) then
  		  	return 0;
  		  end if;
  		end if;  
  	end if;
  end if;	
  
  
  
  return 1;
  
exception
  when others then
  	return 1;

end reraisecheck;

end;
/