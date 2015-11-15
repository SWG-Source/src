// ======================================================================
//
// TaskRenameCharacter.cpp
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#include "FirstLoginServer.h"
#include "TaskRenameCharacter.h"

#include "LoginServer.h"
#include "DatabaseConnection.h"
#include "serverNetworkMessages/TransferCharacterData.h"
#include "serverNetworkMessages/TransferCharacterDataArchive.h"
#include "sharedDatabaseInterface/DbSession.h"
#include "sharedNetworkMessages/GenericValueTypeMessage.h"

// ======================================================================

TaskRenameCharacter::TaskRenameCharacter(uint32 clusterId, const NetworkId &characterId, const Unicode::String &newName, const TransferCharacterData * requestData) :
		TaskRequest(),
		m_clusterId(clusterId),
		m_characterId(characterId),
		m_newName(newName),
		m_success(false),
		m_requestData(0)
{
	if(requestData)
	{
		m_requestData = new TransferCharacterData(*requestData);
	}
}

// ----------------------------------------------------------------------

TaskRenameCharacter::~TaskRenameCharacter()
{
	delete m_requestData;
}

// ----------------------------------------------------------------------

bool TaskRenameCharacter::process(DB::Session *session)
{
	RenameCharacterQuery qry;
	qry.cluster_id=m_clusterId; //lint !e713 loss of precision
	qry.character_id=m_characterId;
	qry.new_name=m_newName;
	
	m_success = session->exec(&qry); 
	
	qry.done();
	return m_success;
}

// ----------------------------------------------------------------------

void TaskRenameCharacter::onComplete()
{
	if(m_requestData)
	{
		m_requestData->setIsValidName(m_success);
		GenericValueTypeMessage<TransferCharacterData> reply("TransferRenameCharacterReplyFromLoginServer", *m_requestData);
		LoginServer::getInstance().sendToCluster(m_clusterId, reply);
	}
}

// ======================================================================

TaskRenameCharacter::RenameCharacterQuery::RenameCharacterQuery() :
		Query(),
		cluster_id(),
		character_id(),
		new_name()
{
}

// ----------------------------------------------------------------------

void TaskRenameCharacter::RenameCharacterQuery::getSQL(std::string &sql)
{
	sql = std::string("begin ")+DatabaseConnection::getInstance().getSchemaQualifier()+"login.rename_character(:cluster_id, :character_id, :new_name); end;";
 	// DEBUG_REPORT_LOG(true, ("TaskRenameCharacter SQL: %s\n", sql.c_str()));
}

// ----------------------------------------------------------------------

bool TaskRenameCharacter::RenameCharacterQuery::bindParameters()
{
	if (!bindParameter(cluster_id)) return false;
	if (!bindParameter(character_id)) return false;
	if (!bindParameter(new_name)) return false;
	return true;
}

// ----------------------------------------------------------------------

bool TaskRenameCharacter::RenameCharacterQuery::bindColumns()
{
	return true;
}

// ----------------------------------------------------------------------

DB::Query::QueryMode TaskRenameCharacter::RenameCharacterQuery::getExecutionMode() const
{
	return MODE_PROCEXEC;
}

// ======================================================================
