// ======================================================================
//
// TaskUpdateSPCharacterProfileData.cpp
// copyright (c) 2005 Sony Online Entertainment
//
// ======================================================================

#include "FirstStationPlayersCollector.h"
#include "TaskUpdateSPCharacterProfileData.h"

#include "StationPlayersCollector.h"
#include "DatabaseConnection.h"
#include "sharedDatabaseInterface/DbSession.h"
#include "sharedFoundation/NetworkIdArchive.h"
#include "ConfigStationPlayersCollector.h"

// ======================================================================

TaskUpdateSPCharacterProfileData::TaskUpdateSPCharacterProfileData(const std::string & clusterName,const NetworkId & characterId,const std::string & characterName,const std::string & objectName,float x,float y,float z,const std::string & sceneId,float cash_balance,float bank_balance,const std::string & objectTemplateName,int   stationId,const NetworkId & containedBy,int   createTime,int   playedTime,int   numLots) :
		TaskRequest(),
		m_clusterName(clusterName),
		m_characterId(characterId),
		m_characterName(characterName),
		m_objectName(objectName),
		m_x(x),
		m_y(y),
		m_z(z),
		m_sceneId(sceneId),
		m_cash_balance(cash_balance),
		m_bank_balance(bank_balance),
		m_objectTemplateName(objectTemplateName),
		m_stationId(stationId),
		m_containedby(containedBy),
		m_createTime(createTime),
		m_playedTime(playedTime),
		m_numLots(numLots)
{
}

// ----------------------------------------------------------------------

bool TaskUpdateSPCharacterProfileData::process(DB::Session *session)
{
	UpdateSPCharacterProfileDataQuery qry;

	qry.clusterName = m_clusterName;
	qry.characterId = m_characterId;
	qry.characterName = m_characterName;
	qry.objectName = m_objectName;
	qry.x = m_x;
	qry.y = m_y;
	qry.z = m_z;
	qry.sceneId = m_sceneId;
	qry.cash_balance = m_cash_balance;
	qry.bank_balance = m_bank_balance;
	qry.objectTemplateName = m_objectTemplateName;
	qry.stationId = static_cast<long>(m_stationId);
	qry.containedBy = m_containedby;
	qry.createTime = m_createTime;
	qry.playedTime = m_playedTime;
	qry.numLots = m_numLots;
	
	bool rval = session->exec(&qry);
	
	qry.done();
	return rval;
}

// ----------------------------------------------------------------------

void TaskUpdateSPCharacterProfileData::onComplete()
{
	// StationPlayersCollector::ackSPCharacterProfileData();
	DEBUG_REPORT_LOG(ConfigStationPlayersCollector::getShowAllDebugInfo(), ("[StationPlayersCollector] : TaskUpdateSPCharacterProfileData::onComplete().\n"));
}

// ======================================================================

TaskUpdateSPCharacterProfileData::UpdateSPCharacterProfileDataQuery::UpdateSPCharacterProfileDataQuery() :
		Query(),
		clusterName(),
		characterId(),
		characterName(),
		objectName(),
		x(),
		y(),
		z(),
		sceneId(),
		cash_balance(),
		bank_balance(),
		objectTemplateName(),
		stationId(),
		containedBy(),
		createTime(),
		playedTime(),
		numLots()
{
}

// ----------------------------------------------------------------------

void TaskUpdateSPCharacterProfileData::UpdateSPCharacterProfileDataQuery::getSQL(std::string &sql)
{
	sql = std::string("begin ")+DatabaseConnection::getSchemaQualifier()+"station_players.update_character(:clusterName,:characterId,:characterName,:objectName,:x,:y,:z,:sceneId,:cash_balance,:bank_balance,:objectTemplateName,:stationId,:containedBy,:createTime,:playedTime,:numLots); end;";
	DEBUG_REPORT_LOG(true, ("TaskUpdateSPCharacterProfileData SQL: %s\n", sql.c_str()));
}

// ----------------------------------------------------------------------

bool TaskUpdateSPCharacterProfileData::UpdateSPCharacterProfileDataQuery::bindParameters()
{
	if (!bindParameter(clusterName)) return false;
	if (!bindParameter(characterId)) return false;
	if (!bindParameter(characterName)) return false;
	if (!bindParameter(objectName)) return false;
	if (!bindParameter(x)) return false;
	if (!bindParameter(y)) return false;
	if (!bindParameter(z)) return false;
	if (!bindParameter(sceneId)) return false;
	if (!bindParameter(cash_balance)) return false;
	if (!bindParameter(bank_balance)) return false;
	if (!bindParameter(objectTemplateName)) return false;
	if (!bindParameter(stationId)) return false;
	if (!bindParameter(containedBy)) return false;
	if (!bindParameter(createTime)) return false;
	if (!bindParameter(playedTime)) return false;
	if (!bindParameter(numLots)) return false;
	
	return true;
}

// ----------------------------------------------------------------------

bool TaskUpdateSPCharacterProfileData::UpdateSPCharacterProfileDataQuery::bindColumns()
{
	return true;
}

// ----------------------------------------------------------------------

DB::Query::QueryMode TaskUpdateSPCharacterProfileData::UpdateSPCharacterProfileDataQuery::getExecutionMode() const
{
	return MODE_PROCEXEC;
}

// ======================================================================
