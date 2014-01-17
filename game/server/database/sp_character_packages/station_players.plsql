create or replace package body station_players
as
	procedure update_character(p_clusterName in varchar2 ,p_characterId in networkid, p_characterName in varchar2, p_objectName in varchar2, p_x in number, p_y in number, p_z in number, p_sceneId in varchar2, p_cash_balance in number, p_bank_balance in number, p_objectTemplateName in varchar2, p_stationId in networkid, p_containedBy in networkid, p_createTime in number, p_playedTime in number , p_numLots in number)
	as
	m_enable_db_logging INTEGER := 0;
	begin
		insert into character_profile (cluster_name, character_id, character_name, object_name, x, y, z, scene_id, cash_balance, bank_balance, object_template_name, station_id, contained_by, create_time, played_time, num_lots)
		values (p_clusterName, p_characterId, p_characterName, p_objectName, p_x, p_y, p_z, p_sceneId, p_cash_balance, p_bank_balance, p_objectTemplateName, p_stationId, p_containedBy, util.unix_time_2_oracle_date_PDT(p_createTime), util.unix_time_2_oracle_date_PDT(p_playedTime), p_numLots);
		
		

	exception
		when dup_val_on_index then
			begin
			
			update character_profile set
			character_name = NVL(p_characterName,character_name),
			object_name = NVL(p_objectName, object_name), 
			x = NVL(p_x, x),
			y = NVL(p_y, y),
			z = NVL(p_z, z),
			scene_id = NVL(p_sceneId, scene_id),
			cash_balance = NVL(p_cash_balance, cash_balance),
			bank_balance = NVL(p_bank_balance, bank_balance),
			object_template_name = NVL(p_objectTemplateName, object_template_name),
			station_id = NVL(p_stationId, station_id),
			contained_by = NVL(p_containedBy, contained_by),
			create_time = NVL(util.unix_time_2_oracle_date_PDT(p_createTime),  create_time),
			played_time = NVL(util.unix_time_2_oracle_date_PDT(p_playedTime),  played_time),
			num_lots = NVL(p_numLots, num_lots)
			where cluster_name = p_clusterName 
			and character_id = p_characterId;
			
			end;
		when others then
			m_enable_db_logging := db_error_logger.getLogLevel();
			IF (m_enable_db_logging > 0) THEN
				db_error_logger.dblogerror(SQLCODE,'station_players.update_character : error.');
					IF (m_enable_db_logging > 1) THEN
						NULL; 
					END IF;
			END IF;
			IF (db_error_logger.reraisecheck('station_players','update_character') = 1) THEN
				RAISE;
			END IF;
	end;
end;
/
