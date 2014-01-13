// ======================================================================
//
// TaskUpgradeAccount.cpp
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#include "FirstLoginServer.h"
#include "TaskUpgradeAccount.h"

#include "LoginServer.h"
#include "DatabaseConnection.h"
#include "serverNetworkMessages/LoginUpgradeAccountMessage.h"
#include "sharedDatabaseInterface/DbSession.h"
#include "sharedFoundation/NetworkIdArchive.h"
#include "sharedNetworkMessages/GenericValueTypeMessage.h"
#include "TaskGetAvatarList.h"

// ======================================================================

TaskUpgradeAccount::TaskUpgradeAccount(LoginUpgradeAccountMessage *message, uint32 clusterId) :
		TaskRequest(),
		m_message(message),
		m_clusterId(clusterId)
{
	NOT_NULL(message);
}

// ----------------------------------------------------------------------

TaskUpgradeAccount::~TaskUpgradeAccount()
{
	delete m_message;
	m_message=0;
}

// ----------------------------------------------------------------------

bool TaskUpgradeAccount::process(DB::Session *session)
{
	bool rval = true;
	switch (m_message->getUpgradeType())
	{
		case LoginUpgradeAccountMessage::UT_addJedi:
		{
			QueryJediQuery qryHasJediSlot;
			qryHasJediSlot.station_id = static_cast<long>(m_message->getStationId());
			qryHasJediSlot.character_type = 2;

			rval = session->exec(&qryHasJediSlot);

			qryHasJediSlot.done();

			// account doesn't have jedi slot yet, add it
			if (!rval || (qryHasJediSlot.result.getValue() == 0))
			{
				AddJediQuery qry;
				qry.cluster_id = static_cast<long>(m_clusterId);
				qry.station_id = static_cast<long>(m_message->getStationId());
				
				rval = session->exec(&qry);
				
				qry.done();
			}
		}
		break;

		case LoginUpgradeAccountMessage::UT_setSpectral:
		{
			SetCharacterTypeQuery qry;
			qry.cluster_id = static_cast<long>(m_clusterId);
			qry.station_id = static_cast<long>(m_message->getStationId());
			qry.character = m_message->getCharacter();
			qry.character_type = 3;
			
			rval = session->exec(&qry);
			
			qry.done();
		}
		break;
		
		default:
			WARNING_STRICT_FATAL(true,("LoginUpgradeAccountMessage contained unrecognized upgrade type.\n"));
	}
	return rval;
	
}

// ----------------------------------------------------------------------

void TaskUpgradeAccount::onComplete()
{
	m_message->setAck();
	LoginServer::getInstance().sendToCluster(m_clusterId,*m_message);
}

// ======================================================================

TaskUpgradeAccount::SetCharacterTypeQuery::SetCharacterTypeQuery() :
		Query(),
		cluster_id(),
		station_id(),
		character(),
		character_type()
{
}

// ----------------------------------------------------------------------

void TaskUpgradeAccount::SetCharacterTypeQuery::getSQL(std::string &sql)
{
	sql = std::string("begin ")+DatabaseConnection::getInstance().getSchemaQualifier()+"login.set_character_type(:cluster_id, :station_id, :character, :character_type); end;";
 	// DEBUG_REPORT_LOG(true, ("TaskUpgradeAccount SQL: %s\n", sql.c_str()));
}

// ----------------------------------------------------------------------

bool TaskUpgradeAccount::SetCharacterTypeQuery::bindParameters()
{
	if (!bindParameter(cluster_id)) return false;
	if (!bindParameter(station_id)) return false;
	if (!bindParameter(character)) return false;
	if (!bindParameter(character_type)) return false;
	return true;
}

// ----------------------------------------------------------------------

bool TaskUpgradeAccount::SetCharacterTypeQuery::bindColumns()
{
	return true;
}

// ----------------------------------------------------------------------

DB::Query::QueryMode TaskUpgradeAccount::SetCharacterTypeQuery::getExecutionMode() const
{
	return MODE_PROCEXEC;
}

// ======================================================================

TaskUpgradeAccount::AddJediQuery::AddJediQuery() :
		Query(),
		cluster_id(),
		station_id()
{
}

// ----------------------------------------------------------------------

void TaskUpgradeAccount::AddJediQuery::getSQL(std::string &sql)
{
	sql = std::string("begin ")+DatabaseConnection::getInstance().getSchemaQualifier()+"login.set_character_slots(:cluster_id, :station_id, 2, 1); end;";
}

// ----------------------------------------------------------------------

bool TaskUpgradeAccount::AddJediQuery::bindParameters()
{
	if (!bindParameter(cluster_id)) return false;
	if (!bindParameter(station_id)) return false;
	return true;
}

// ----------------------------------------------------------------------

bool TaskUpgradeAccount::AddJediQuery::bindColumns()
{
	return true;
}

// ----------------------------------------------------------------------

DB::Query::QueryMode TaskUpgradeAccount::AddJediQuery::getExecutionMode() const
{
	return MODE_PROCEXEC;
}

// ======================================================================

TaskUpgradeAccount::QueryJediQuery::QueryJediQuery() :
Query(),
station_id(),
character_type(),
result()
{
}

// ----------------------------------------------------------------------

void TaskUpgradeAccount::QueryJediQuery::getSQL(std::string &sql)
{
	sql = std::string("begin :result := ")+DatabaseConnection::getInstance().getSchemaQualifier()+"login.has_extra_character_slot(:station_id, :character_type); end;";
}

// ----------------------------------------------------------------------

bool TaskUpgradeAccount::QueryJediQuery::bindParameters()
{
	if (!bindParameter(result)) return false;
	if (!bindParameter(station_id)) return false;
	if (!bindParameter(character_type)) return false;
	return true;
}

// ----------------------------------------------------------------------

bool TaskUpgradeAccount::QueryJediQuery::bindColumns()
{
	return true;
}

// ----------------------------------------------------------------------

DB::Query::QueryMode TaskUpgradeAccount::QueryJediQuery::getExecutionMode() const
{
	return MODE_PROCEXEC;
}

// ======================================================================

TaskOccupyUnlockedSlot::TaskOccupyUnlockedSlot(int clusterGroupId, uint32 clusterId, StationId stationId, NetworkId const & characterId, uint32 replyGameServerId) :
TaskRequest(),
m_clusterGroupId(clusterGroupId),
m_clusterId(clusterId),
m_stationId(stationId),
m_characterId(characterId),
m_replyGameServerId(replyGameServerId),
m_result(static_cast<int>(LoginUpgradeAccountMessage::OUSR_db_error))
{
}

// ----------------------------------------------------------------------

TaskOccupyUnlockedSlot::~TaskOccupyUnlockedSlot()
{
}

// ----------------------------------------------------------------------

bool TaskOccupyUnlockedSlot::process(DB::Session *session)
{
	bool rval = true;
	m_result = static_cast<int>(LoginUpgradeAccountMessage::OUSR_success);

	// see if account has an unlocked slot
	TaskUpgradeAccount::QueryJediQuery qryHasJediSlot;
	qryHasJediSlot.station_id = static_cast<long>(m_stationId);
	qryHasJediSlot.character_type = 2;

	rval = session->exec(&qryHasJediSlot);

	qryHasJediSlot.done();

	if (!rval)
	{
		m_result = static_cast<int>(LoginUpgradeAccountMessage::OUSR_db_error);
		return false;
	}

	// account doesn't have unlocked slot
	int const numberUnlockedSlot = static_cast<int>(qryHasJediSlot.result.getValue());
	if (numberUnlockedSlot == 0)
	{
		m_result = static_cast<int>(LoginUpgradeAccountMessage::OUSR_account_has_no_unlocked_slot);
		return true;
	}

	// get the list of characters for the account to see if the account already has an unlocked slot character
	TaskGetAvatarList::GetCharactersQuery qryGetCharacters;

	qryGetCharacters.station_id = m_stationId; //lint !e713 // loss of precision unsigned long to long
	qryGetCharacters.cluster_group_id = m_clusterGroupId;

	rval = session->exec(&qryGetCharacters);
	if (!rval)
	{
		m_result = static_cast<int>(LoginUpgradeAccountMessage::OUSR_db_error);
		return false;
	}

	int numberUnlockedSlotCharacter = 0;
	bool hasUnlockedSlotCharacterOnCluster = false;
	int rowsFetched;
	while ((rowsFetched = qryGetCharacters.fetch()) > 0)
	{
		if (qryGetCharacters.character_type.getValue() == 2)
		{
			++numberUnlockedSlotCharacter;
			
			if (qryGetCharacters.cluster_id.getValue() == static_cast<long>(m_clusterId))
				hasUnlockedSlotCharacterOnCluster = true;
		}
	}

	qryGetCharacters.done();

	if (numberUnlockedSlotCharacter >= numberUnlockedSlot)
	{
		m_result = static_cast<int>(LoginUpgradeAccountMessage::OUSR_account_has_no_unoccupied_unlocked_slot);
		return true;
	}

	if (hasUnlockedSlotCharacterOnCluster)
	{
		m_result = static_cast<int>(LoginUpgradeAccountMessage::OUSR_cluster_already_has_unlocked_slot_character);
		return true;
	}

	// make the character an unlocked slot character
	TaskUpgradeAccount::SetCharacterTypeQuery qrySetUnlocked;
	qrySetUnlocked.cluster_id = static_cast<long>(m_clusterId);
	qrySetUnlocked.station_id = static_cast<long>(m_stationId);
	qrySetUnlocked.character = m_characterId;
	qrySetUnlocked.character_type = 2;

	rval = session->exec(&qrySetUnlocked);

	qrySetUnlocked.done();

	if (!rval)
	{
		m_result = static_cast<int>(LoginUpgradeAccountMessage::OUSR_db_error);
		return false;
	}

	return true;
}

// ----------------------------------------------------------------------

void TaskOccupyUnlockedSlot::onComplete()
{
	GenericValueTypeMessage<std::pair<std::pair<int, NetworkId>, uint32> > const occupyUnlockedSlotRsp("OccupyUnlockedSlotRsp", std::make_pair(std::make_pair(m_result, m_characterId), m_replyGameServerId));
	LoginServer::getInstance().sendToCluster(m_clusterId, occupyUnlockedSlotRsp);
}

// ======================================================================

TaskVacateUnlockedSlot::TaskVacateUnlockedSlot(int clusterGroupId, uint32 clusterId, StationId stationId, NetworkId const & characterId, uint32 replyGameServerId) :
TaskRequest(),
m_clusterGroupId(clusterGroupId),
m_clusterId(clusterId),
m_stationId(stationId),
m_characterId(characterId),
m_replyGameServerId(replyGameServerId),
m_result(static_cast<int>(LoginUpgradeAccountMessage::VUSR_db_error))
{
}

// ----------------------------------------------------------------------

TaskVacateUnlockedSlot::~TaskVacateUnlockedSlot()
{
}

// ----------------------------------------------------------------------

bool TaskVacateUnlockedSlot::process(DB::Session *session)
{
	bool rval = true;
	m_result = static_cast<int>(LoginUpgradeAccountMessage::VUSR_success);

	// see if account has an unlocked slot
	TaskUpgradeAccount::QueryJediQuery qryHasJediSlot;
	qryHasJediSlot.station_id = static_cast<long>(m_stationId);
	qryHasJediSlot.character_type = 2;

	rval = session->exec(&qryHasJediSlot);

	qryHasJediSlot.done();

	if (!rval)
	{
		m_result = static_cast<int>(LoginUpgradeAccountMessage::VUSR_db_error);
		return false;
	}

	// account doesn't have unlocked slot
	if (qryHasJediSlot.result.getValue() == 0)
	{
		m_result = static_cast<int>(LoginUpgradeAccountMessage::VUSR_account_has_no_unlocked_slot);
		return true;
	}

	// get the list of characters for the account to see if this character is an unlocked slot character
	TaskGetAvatarList::GetCharactersQuery qryGetCharacters;

	qryGetCharacters.station_id = m_stationId; //lint !e713 // loss of precision unsigned long to long
	qryGetCharacters.cluster_group_id = m_clusterGroupId;

	rval = session->exec(&qryGetCharacters);
	if (!rval)
	{
		m_result = static_cast<int>(LoginUpgradeAccountMessage::VUSR_db_error);
		return false;
	}

	bool isUnlockedSlotCharacter = false;
	int rowsFetched;
	while ((rowsFetched = qryGetCharacters.fetch()) > 0)
	{
		if ((qryGetCharacters.cluster_id.getValue() == static_cast<long>(m_clusterId)) && (qryGetCharacters.object_id.getValue() == m_characterId))
		{
			if (qryGetCharacters.character_type.getValue() == 2)
			{
				isUnlockedSlotCharacter = true;
			}

			break;
		}
	}

	qryGetCharacters.done();

	if (!isUnlockedSlotCharacter)
	{
		m_result = static_cast<int>(LoginUpgradeAccountMessage::VUSR_not_unlocked_slot_character);
		return true;
	}

	// check to see if the account can create a normal character, because
	// we are going to be making the character into a normal character, and
	// we must be sure that doing will not violate the normal character limit
	GetOnlyOpenCharacterSlotsQuery qryGetOnlyOpenCharacterSlots;
	qryGetOnlyOpenCharacterSlots.station_id = static_cast<long>(m_stationId);
	qryGetOnlyOpenCharacterSlots.cluster_id = static_cast<long>(m_clusterId);

	rval = session->exec(&qryGetOnlyOpenCharacterSlots);
	if (!rval)
	{
		m_result = static_cast<int>(LoginUpgradeAccountMessage::VUSR_db_error);
		return false;
	}

	std::vector<int> openCharacterSlots(4,0);
	while ((rowsFetched = qryGetOnlyOpenCharacterSlots.fetch()) > 0)
	{
		openCharacterSlots[static_cast<unsigned long>(qryGetOnlyOpenCharacterSlots.character_type_id.getValue())]=qryGetOnlyOpenCharacterSlots.num_open_slots.getValue();
	}

	qryGetOnlyOpenCharacterSlots.done();

	if (openCharacterSlots[1] <= 0)
	{
		m_result = static_cast<int>(LoginUpgradeAccountMessage::VUSR_no_available_normal_character_slot);
		return true;
	}

	// make the character a normal slot character
	TaskUpgradeAccount::SetCharacterTypeQuery qrySetNormal;
	qrySetNormal.cluster_id = static_cast<long>(m_clusterId);
	qrySetNormal.station_id = static_cast<long>(m_stationId);
	qrySetNormal.character = m_characterId;
	qrySetNormal.character_type = 1;

	rval = session->exec(&qrySetNormal);

	qrySetNormal.done();

	if (!rval)
	{
		m_result = static_cast<int>(LoginUpgradeAccountMessage::VUSR_db_error);
		return false;
	}

	return true;
}

// ----------------------------------------------------------------------

void TaskVacateUnlockedSlot::onComplete()
{
	GenericValueTypeMessage<std::pair<std::pair<int, NetworkId>, std::pair<uint32, uint32> > > const vacateUnlockedSlotRsp("VacateUnlockedSlotRsp", std::make_pair(std::make_pair(m_result, m_characterId), std::make_pair(static_cast<uint32>(m_stationId), m_replyGameServerId)));
	LoginServer::getInstance().sendToCluster(m_clusterId, vacateUnlockedSlotRsp);
}

// ======================================================================

TaskVacateUnlockedSlot::GetOnlyOpenCharacterSlotsQuery::GetOnlyOpenCharacterSlotsQuery() :
Query(),
station_id(),
cluster_id(),
character_type_id(),
num_open_slots()
{
}

// ----------------------------------------------------------------------

void TaskVacateUnlockedSlot::GetOnlyOpenCharacterSlotsQuery::getSQL(std::string &sql)
{
	sql = std::string("begin :rc := ")+DatabaseConnection::getInstance().getSchemaQualifier()+"login.get_only_open_character_slots(:station_id,:cluster_id); end;";
}

// ----------------------------------------------------------------------

bool TaskVacateUnlockedSlot::GetOnlyOpenCharacterSlotsQuery::bindParameters()
{
	if (!bindParameter(station_id)) return false;
	if (!bindParameter(cluster_id)) return false;

	return true;	
}

// ----------------------------------------------------------------------

bool TaskVacateUnlockedSlot::GetOnlyOpenCharacterSlotsQuery::bindColumns()
{
	if (!bindCol(character_type_id)) return false;
	if (!bindCol(num_open_slots)) return false;

	return true;
}

// ----------------------------------------------------------------------

DB::Query::QueryMode TaskVacateUnlockedSlot::GetOnlyOpenCharacterSlotsQuery::getExecutionMode() const
{
	return MODE_PLSQL_REFCURSOR;
}

// ======================================================================

TaskSwapUnlockedSlot::TaskSwapUnlockedSlot(int clusterGroupId, uint32 clusterId, StationId stationId, NetworkId const & sourceCharacterId, NetworkId const & targetCharacterId, uint32 replyGameServerId) :
TaskRequest(),
m_clusterGroupId(clusterGroupId),
m_clusterId(clusterId),
m_stationId(stationId),
m_sourceCharacterId(sourceCharacterId),
m_targetCharacterId(targetCharacterId),
m_targetCharacterName(),
m_replyGameServerId(replyGameServerId),
m_result(static_cast<int>(LoginUpgradeAccountMessage::SUSR_db_error))
{
}

// ----------------------------------------------------------------------

TaskSwapUnlockedSlot::~TaskSwapUnlockedSlot()
{
}

// ----------------------------------------------------------------------

bool TaskSwapUnlockedSlot::process(DB::Session *session)
{
	bool rval = true;
	m_result = static_cast<int>(LoginUpgradeAccountMessage::SUSR_success);

	// see if account has an unlocked slot
	TaskUpgradeAccount::QueryJediQuery qryHasJediSlot;
	qryHasJediSlot.station_id = static_cast<long>(m_stationId);
	qryHasJediSlot.character_type = 2;

	rval = session->exec(&qryHasJediSlot);

	qryHasJediSlot.done();

	if (!rval)
	{
		m_result = static_cast<int>(LoginUpgradeAccountMessage::SUSR_db_error);
		return false;
	}

	// account doesn't have unlocked slot
	if (qryHasJediSlot.result.getValue() == 0)
	{
		m_result = static_cast<int>(LoginUpgradeAccountMessage::SUSR_account_has_no_unlocked_slot);
		return true;
	}

	// get the list of characters for the account to see if this character is an unlocked slot character,
	// and the the target character is not an unlocked slot character, and the target character is on
	// the same cluster
	TaskGetAvatarList::GetCharactersQuery qryGetCharacters;

	qryGetCharacters.station_id = m_stationId; //lint !e713 // loss of precision unsigned long to long
	qryGetCharacters.cluster_group_id = m_clusterGroupId;

	rval = session->exec(&qryGetCharacters);
	if (!rval)
	{
		m_result = static_cast<int>(LoginUpgradeAccountMessage::SUSR_db_error);
		return false;
	}

	bool foundSourceCharacter = false;
	bool isSourceUnlockedSlotCharacter = false;
	bool isTargetUnlockedSlotCharacter = false;
	bool isTargetOnSameCluster = false;
	int rowsFetched;
	while ((rowsFetched = qryGetCharacters.fetch()) > 0)
	{
		if ((qryGetCharacters.cluster_id.getValue() == static_cast<long>(m_clusterId)) && (qryGetCharacters.object_id.getValue() == m_sourceCharacterId))
		{
			foundSourceCharacter = true;

			if (qryGetCharacters.character_type.getValue() == 2)
			{
				isSourceUnlockedSlotCharacter = true;
			}
		}

		// this is not a "else if" because we don't trust that m_sourceCharacterId is always different from m_targetCharacterId
		if ((qryGetCharacters.cluster_id.getValue() == static_cast<long>(m_clusterId)) && (qryGetCharacters.object_id.getValue() == m_targetCharacterId))
		{
			qryGetCharacters.character_name.getValue(m_targetCharacterName);
			isTargetOnSameCluster = true;

			if (qryGetCharacters.character_type.getValue() == 2)
			{
				isTargetUnlockedSlotCharacter = true;
			}
		}

		if (foundSourceCharacter && isTargetOnSameCluster)
			break;
	}

	qryGetCharacters.done();

	if (!isSourceUnlockedSlotCharacter)
	{
		m_result = static_cast<int>(LoginUpgradeAccountMessage::SUSR_not_unlocked_slot_character);
		return true;
	}

	if (!isTargetOnSameCluster)
	{
		m_result = static_cast<int>(LoginUpgradeAccountMessage::SUSR_invalid_target_character);
		return true;
	}

	if (isTargetUnlockedSlotCharacter)
	{
		m_result = static_cast<int>(LoginUpgradeAccountMessage::SUSR_target_character_already_unlocked_slot_character);
		return true;
	}

	// make the source character a normal slot character
	TaskUpgradeAccount::SetCharacterTypeQuery qrySetNormal;
	qrySetNormal.cluster_id = static_cast<long>(m_clusterId);
	qrySetNormal.station_id = static_cast<long>(m_stationId);
	qrySetNormal.character = m_sourceCharacterId;
	qrySetNormal.character_type = 1;

	rval = session->exec(&qrySetNormal);

	qrySetNormal.done();

	if (!rval)
	{
		m_result = static_cast<int>(LoginUpgradeAccountMessage::SUSR_db_error);
		return false;
	}

	// make the target character an unlocked slot character
	TaskUpgradeAccount::SetCharacterTypeQuery qrySetUnlocked;
	qrySetUnlocked.cluster_id = static_cast<long>(m_clusterId);
	qrySetUnlocked.station_id = static_cast<long>(m_stationId);
	qrySetUnlocked.character = m_targetCharacterId;
	qrySetUnlocked.character_type = 2;

	rval = session->exec(&qrySetUnlocked);

	qrySetUnlocked.done();

	if (!rval)
	{
		m_result = static_cast<int>(LoginUpgradeAccountMessage::SUSR_db_error);
		return false;
	}

	return true;
}

// ----------------------------------------------------------------------

void TaskSwapUnlockedSlot::onComplete()
{
	GenericValueTypeMessage<std::pair<std::pair<int, NetworkId>, std::pair<uint32, std::pair<NetworkId, std::string> > > > const swapUnlockedSlotRsp("SwapUnlockedSlotRsp", std::make_pair(std::make_pair(m_result, m_sourceCharacterId), std::make_pair(m_replyGameServerId, std::make_pair(m_targetCharacterId, m_targetCharacterName))));
	LoginServer::getInstance().sendToCluster(m_clusterId, swapUnlockedSlotRsp);
}

// ======================================================================
