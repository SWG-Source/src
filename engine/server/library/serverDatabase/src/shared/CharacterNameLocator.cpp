// ======================================================================
//
// CharacterNameLocator.cpp
// copyright (c) 2003 Sony Online Entertainment
//
// ======================================================================

#include "serverDatabase/FirstServerDatabase.h"
#include "serverDatabase/CharacterNameLocator.h"

#include "serverDatabase/ConfigServerDatabase.h"
#include "serverDatabase/DatabaseProcess.h"
#include "serverDatabase/GameServerConnection.h"
#include "serverNetworkMessages/CharacterNamesMessage.h"
#include "sharedDatabaseInterface/DbSession.h"

// ======================================================================

bool CharacterNameLocator::locateObjects(DB::Session *session, const std::string &schema, int &objectsLocated)
{
	int rowsFetched;
	DEBUG_REPORT_LOG(ConfigServerDatabase::getLogObjectLoading(),("Loading character names.\n"));
	
	CharacterNameQuery qry(schema);
	if (! (session->exec(&qry)))
	{
		objectsLocated = 0;
		return false;
	}
	while ((rowsFetched = qry.fetch()) > 0)
	{
		size_t numRows = qry.getNumRowsFetched();
		size_t count = 0;
		const std::vector<CharacterNameRow> &data = qry.getData();

		for (std::vector<CharacterNameRow>::const_iterator i=data.begin(); i!=data.end(); ++i)
		{
			if (++count > numRows)
				break;

			m_characterIds.push_back(i->character_id.getValue());
			m_stationIds.push_back(i->character_station_id.getValue());
			m_characterNames.push_back(i->character_name.getValueASCII());
			m_characterFullNames.push_back(i->character_full_name.getValueASCII());
			m_characterCreateTime.push_back(i->character_create_time.getValue() + (7 * 60 * 60));			
			m_characterLastLoginTime.push_back(i->character_last_login_time.getValue() + (7 * 60 * 60));
			DEBUG_REPORT_LOG(ConfigServerDatabase::getLogObjectLoading(),("\t%s:%s\n", i->character_id.getValue().getValueString().c_str(), i->character_name.getValueASCII().c_str()));
		}
	}

	objectsLocated = 0;
	return (rowsFetched >= 0);
}

// ----------------------------------------------------------------------

void CharacterNameLocator::sendPostBaselinesCustomData(GameServerConnection &conn) const
{
	CharacterNamesMessage msg(m_characterIds, m_stationIds, m_characterNames, m_characterFullNames, m_characterCreateTime, m_characterLastLoginTime);
	conn.send(msg,true);
}

// ======================================================================

CharacterNameLocator::CharacterNameQuery::CharacterNameQuery(const std::string &schema) :
		m_data(ConfigServerDatabase::getDefaultFetchBatchSize()),
		m_schema(schema)
{
}

// ----------------------------------------------------------------------

void CharacterNameLocator::CharacterNameQuery::getSQL(std::string &sql)
{
	sql="begin :result := "+m_schema+"loader.get_character_name_list(); end;";
}

// ----------------------------------------------------------------------

bool CharacterNameLocator::CharacterNameQuery::bindParameters()
{
	return true;
}

// ----------------------------------------------------------------------

bool CharacterNameLocator::CharacterNameQuery::bindColumns()
{
	size_t skipSize = reinterpret_cast<char*>(&(m_data[1])) - reinterpret_cast<char*>(&(m_data[0]));
	setColArrayMode(skipSize, m_data.size());

	if (!bindCol(m_data[0].character_id)) return false;
	if (!bindCol(m_data[0].character_station_id)) return false;
	if (!bindCol(m_data[0].character_name)) return false;
	if (!bindCol(m_data[0].character_full_name)) return false;
	if (!bindCol(m_data[0].character_create_time)) return false;
	if (!bindCol(m_data[0].character_last_login_time)) return false;
	return true;
}

// ----------------------------------------------------------------------

DB::Query::QueryMode CharacterNameLocator::CharacterNameQuery::getExecutionMode() const
{
	return MODE_PLSQL_REFCURSOR;
}

// ======================================================================
