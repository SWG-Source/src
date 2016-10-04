// ======================================================================
//
// TaskRestoreCharacter.cpp
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#include "SwgDatabaseServer/FirstSwgDatabaseServer.h"
#include "TaskRestoreCharacter.h"

#include "serverDatabase/DatabaseProcess.h"
#include "sharedDatabaseInterface/DbSession.h"
#include "sharedLog/Log.h"
#include "sharedNetworkMessages/GenericValueTypeMessage.h"
#include "serverNetworkMessages/LoginRestoreCharacterMessage.h"
#include "unicodeArchive/UnicodeArchive.h"

// ======================================================================

TaskRestoreCharacter::TaskRestoreCharacter(const NetworkId &characterId, const std::string &whoRequested) :
	TaskRequest(),
	m_characterId(characterId),
	m_whoRequested(whoRequested),
	m_result(0),
	m_account(0),
	m_templateId(0)
{
}

//-----------------------------------------------------------------------

TaskRestoreCharacter::~TaskRestoreCharacter()
{
}

//-----------------------------------------------------------------------

bool TaskRestoreCharacter::process(DB::Session *session)
{
	RestoreCharacterQuery query;
	query.character_id = m_characterId;

	if (!(session->exec(&query)))
		return false;
	query.done();

	query.result.getValue(m_result);
	query.character_name.getValue(m_characterName);
	query.account.getValue(m_account);
	query.template_id.getValue(m_templateId);

	LOG("DatabaseRestore", ("Restore Character (%s): result %i", m_characterId.getValueString().c_str(), m_result));
	return true;
}

//-----------------------------------------------------------------------

void TaskRestoreCharacter::onComplete()
{
	if (m_result == 1)
	{
		LoginRestoreCharacterMessage msg(m_whoRequested, m_characterId, m_account, m_characterName, m_templateId, false); //TODO:  "/restoreJedi" command
		DatabaseProcess::getInstance().sendToCentralServer(msg, true);
	}
	else
	{
		std::string message;
		switch (m_result)
		{
		case 2:
			message = "Object id was incorrect or character was not deleted";
			break;

		case 3:
			message = "There was an error in the database while attempting to restore the character.";
			break;

		default:
			message = "The database returned an unknown code in response to the request to restore the character.";
			break;
		}

		GenericValueTypeMessage<std::pair<std::string, std::string> > reply("DatabaseConsoleReplyMessage",
			std::make_pair(m_whoRequested, message));
		DatabaseProcess::getInstance().sendToAnyGameServer(reply);
	}
}

// ======================================================================

void TaskRestoreCharacter::RestoreCharacterQuery::getSQL(std::string &sql)
{
	sql = std::string("begin :result := ") + DatabaseProcess::getInstance().getSchemaQualifier() + "admin.restore_character (:character_id, :character_name, :account, :template_id); end;";
}

// ----------------------------------------------------------------------

bool TaskRestoreCharacter::RestoreCharacterQuery::bindParameters()
{
	if (!bindParameter(result)) return false;
	if (!bindParameter(character_id)) return false;
	if (!bindParameter(character_name)) return false;
	if (!bindParameter(account)) return false;
	if (!bindParameter(template_id)) return false;

	return true;
}

// ----------------------------------------------------------------------

bool TaskRestoreCharacter::RestoreCharacterQuery::bindColumns()
{
	return true;
}

// ----------------------------------------------------------------------

DB::Query::QueryMode TaskRestoreCharacter::RestoreCharacterQuery::getExecutionMode() const
{
	return MODE_PROCEXEC;
}

// ----------------------------------------------------------------------

TaskRestoreCharacter::RestoreCharacterQuery::RestoreCharacterQuery()
{
}

// ======================================================================