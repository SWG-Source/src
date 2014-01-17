create or replace package body biography
as
	procedure set_biography (p_owner objectid, p_biography varchar2)
	as
	
	m_enable_db_logging INTEGER := 0;
	
	begin
		if (p_biography is not null) then
			update biographies
			set biography = p_biography
			where object_id = p_owner;

			if (sql%rowcount = 0) then
				insert into biographies (object_id, biography)
				values (p_owner, p_biography);
			end if;
		else
			delete biographies
			where object_id = p_owner;
		end if;
	exception
		when others then
			begin
				m_enable_db_logging := db_error_logger.getLogLevel();
				IF (m_enable_db_logging > 0) THEN
					db_error_logger.dblogerror(SQLCODE,'biography.set_biography : update error.');
					IF (m_enable_db_logging > 1) THEN
						db_error_logger.dblogerror_values('biography.set_biography','objectid','number',p_owner);
						db_error_logger.dblogerror_values('biography.set_biography','biography','varchar2',p_biography);
					END IF;
				END IF;
				IF (db_error_logger.reraisecheck('biography','set_biography') = 1) THEN
					-- RAISE;
					NULL;
				END IF;
			end;	
	end;

	function get_biography (p_owner objectid) return refcursor
	as
		rc refcursor;
	begin
		open rc for 
		select biography
		from biographies
		where object_id = p_owner;

		return rc;
	end;
end;
/
