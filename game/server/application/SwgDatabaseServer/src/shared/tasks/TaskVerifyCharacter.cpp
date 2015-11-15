// ======================================================================
//
// TaskVerifyCharacter.cpp
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#include "SwgDatabaseServer/FirstSwgDatabaseServer.h"
#include "TaskVerifyCharacter.h"

#include "SwgLoader.h"
#include "Unicode.h"
#include "serverDatabase/CharacterQueries.h"
#include "serverDatabase/DatabaseProcess.h"
#include "serverNetworkMessages/CharacterListMessage.h"
#include "serverNetworkMessages/TransferCharacterData.h"
#include "serverNetworkMessages/TransferCharacterDataArchive.h"
#include "serverNetworkMessages/ValidateCharacterForLoginReplyMessage.h"
#include "sharedDatabaseInterface/DbSession.h"
#include "sharedNetworkMessages/GenericValueTypeMessage.h"
#include "sharedLog/Log.h"
#include "unicodeArchive/UnicodeArchive.h"

// ======================================================================

TaskVerifyCharacter::TaskVerifyCharacter(const std::string &schema) :
		TaskRequest(),
		m_characters(),
		m_goldSchema(schema)
{
	LOG("TRACE_LOGIN", ("Create DB TaskVerifyCharacter"));
}
	
//-----------------------------------------------------------------------

TaskVerifyCharacter::~TaskVerifyCharacter()
{
}

//-----------------------------------------------------------------------

bool TaskVerifyCharacter::process(DB::Session *session)
{
	LOG("TRACE_LOGIN", ("TaskVerifyCharacter -- starting process"));

	VerifyCharacterQuery query;
	query.gold_schema.setValue(m_goldSchema);
	
	for (std::vector<CharacterRecord*>::iterator i=m_characters.begin(); i!=m_characters.end(); ++i)
	{
		query.station_id = static_cast<long>((*i)->m_suid);
		query.character_id = (*i)->m_characterId;
		LOG("TRACE_LOGIN", ("TaskVerifyCharacter(%d, %s) -- processing", (*i)->m_suid, (*i)->m_characterId.getValueString().c_str()));

		if(session->exec(&query))
		{
			(*i)->m_characterName = query.character_name.getValue();
			(*i)->m_approved = query.approved.getValue();
			query.scene_id.getValue((*i)->m_sceneId);
			(*i)->m_containerId = query.container_id.getValue();
			(*i)->m_x = static_cast<float>(query.x.getValue());
			(*i)->m_y = static_cast<float>(query.y.getValue());
			(*i)->m_z = static_cast<float>(query.z.getValue());
			query.containment_flag.getValue((*i)->m_containment_flag);
		}
		else
		{
			return false;
		}
	}
		
	query.done();

	LOG("TRACE_LOGIN", ("TaskVerifyCharacter -- finished process"));
	return true;
}

//-----------------------------------------------------------------------

void TaskVerifyCharacter::onComplete()
{
//	REPORT_LOG(true, ("TaskVerifyCharacter::onComplete()\n"));
	for (std::vector<CharacterRecord*>::iterator i=m_characters.begin(); i!=m_characters.end(); ++i)
	{
		if(! (*i)->m_transferCharacterData)
		{
			ValidateCharacterForLoginReplyMessage msg(
				(*i)->m_approved,
				(*i)->m_suid,
				(*i)->m_characterId,
				(*i)->m_containerId,
				(*i)->m_sceneId,
				Vector((*i)->m_x,(*i)->m_y,(*i)->m_z),
				(*i)->m_characterName);
			DatabaseProcess::getInstance().sendToCentralServer(msg,true);
			if ((*i)->m_approved)
				LOG("TRACE_LOGIN", ("TaskVerifyCharacter(%d, %s) completed -- success", (*i)->m_suid, (*i)->m_characterId.getValueString().c_str()));
			else
				LOG("TRACE_LOGIN", ("TaskVerifyCharacter(%d, %s) completed -- failure", (*i)->m_suid, (*i)->m_characterId.getValueString().c_str()));
			
			// 0 = there was a database error trying to fix containment
			// 1 = there is a recursive containment error in the character object chain that wasn't fixed
			// 2 = there is a recursive containment error in the containment chain not in the character chain
			// 3 = the containment chain is ok
			// 4 = there was a recursive containment error in the character object chain that was fixed	
			switch ((*i)->m_containment_flag )
			{
				case 0: LOG("TRACE_LOGIN", ("TaskVerifyCharacter(%d, %s) completed -- containment error (database error verifying containment)", (*i)->m_suid, (*i)->m_characterId.getValueString().c_str()));
				
					LOG("CustomerService", ("Login: TaskVerifyCharacter(%d, %s) completed -- containment error (database error verifying containment)", (*i)->m_suid, (*i)->m_characterId.getValueString().c_str()));
				break;
				
				case 1: LOG("TRACE_LOGIN", ("TaskVerifyCharacter(%d, %s) completed -- containment error (recursive containment error in the character object that wasn't fixed)", (*i)->m_suid, (*i)->m_characterId.getValueString().c_str()));
				
					LOG("CustomerService", ("Login: TaskVerifyCharacter(%d, %s) completed -- containment error (recursive containment error in the character object that wasn't fixed)", (*i)->m_suid, (*i)->m_characterId.getValueString().c_str()));
				break;  
			
				case 2: LOG("TRACE_LOGIN", ("TaskVerifyCharacter(%d, %s) completed -- containment error (recursive containment error not in the character chain that must be fix manually)", (*i)->m_suid, (*i)->m_characterId.getValueString().c_str()));
				
					LOG("CustomerService", ("Login: TaskVerifyCharacter(%d, %s) completed -- containment error (recursive containment error not in the character chain that must be fix manually)", (*i)->m_suid, (*i)->m_characterId.getValueString().c_str()));
				break;  
			
				case 3:  //containment was OK so we don't need to log it
				break;  
				
				case 4: LOG("TRACE_LOGIN", ("TaskVerifyCharacter(%d, %s) completed -- containment error (recursive containment error in the character object that was fixed)", (*i)->m_suid, (*i)->m_characterId.getValueString().c_str()));
				
					LOG("CustomerService", ("Login: TaskVerifyCharacter(%d, %s) completed -- containment error (recursive containment error in the character object that was fixed)", (*i)->m_suid, (*i)->m_characterId.getValueString().c_str()));
				break;  
				
				default:
					LOG("TRACE_LOGIN", ("TaskVerifyCharacter(%d, %s) completed -- containment error (unknown containment flag)", (*i)->m_suid, (*i)->m_characterId.getValueString().c_str()));
					
					LOG("CustomerService", ("Login: TaskVerifyCharacter(%d, %s) completed -- containment error (unknown containment flag)", (*i)->m_suid, (*i)->m_characterId.getValueString().c_str()));
					
					break;
				
			}
		}
		else
		{
			const TransferCharacterData * d = (*i)->m_transferCharacterData;
			TransferCharacterData responseData(*d);
			responseData.setCharacterId((*i)->m_characterId);
			responseData.setContainerId((*i)->m_containerId);
			responseData.setScene((*i)->m_sceneId);
			responseData.setStartingCoordinates(Vector((*i)->m_x, (*i)->m_y, (*i)->m_z));
			GenericValueTypeMessage<TransferCharacterData> response("TransferReplyLoginLocationData", responseData); // starting coordinates
			DatabaseProcess::getInstance().sendToCentralServer(response, true);
			LOG("CustomerService", ("CharacterTransfer: ***SwgDatabaseServer: sending TransferReplyLoginLocationData to central server for character %s\n", responseData.toString().c_str()));
		}

		delete *i;
	}
	m_characters.clear();
	
	dynamic_cast<SwgLoader&>(SwgLoader::getInstance()).verifyCharacterFinished(this);
}

// ======================================================================

void TaskVerifyCharacter::VerifyCharacterQuery::getSQL(std::string &sql)
{
	// Note:  station id and character id are input parameters.  The rest are pass-by-reference output parameters.
	sql=std::string("begin ") + DatabaseProcess::getInstance().getSchemaQualifier()+"loader.verify_character (:station_id, :character_id, :gold_schema, :approved, :character_name, :scene_id, :container_id, :x, :y, :z, :containment_flag); end;";
}

// ----------------------------------------------------------------------

bool TaskVerifyCharacter::VerifyCharacterQuery::bindParameters()
{
	if (!bindParameter(station_id)) return false;
	if (!bindParameter(character_id)) return false;
	if (!bindParameter(gold_schema)) return false;

	if (!bindParameter(approved)) return false;
	if (!bindParameter(character_name)) return false;
	if (!bindParameter(scene_id)) return false;
	if (!bindParameter(container_id)) return false;
	if (!bindParameter(x)) return false;
	if (!bindParameter(y)) return false;
	if (!bindParameter(z)) return false;
	if (!bindParameter(containment_flag)) return false;
	
	return true;
}

// ----------------------------------------------------------------------

bool TaskVerifyCharacter::VerifyCharacterQuery::bindColumns()
{
	return true;
}

// ----------------------------------------------------------------------

DB::Query::QueryMode TaskVerifyCharacter::VerifyCharacterQuery::getExecutionMode() const
{
	return MODE_PROCEXEC;
}

// ----------------------------------------------------------------------

TaskVerifyCharacter::VerifyCharacterQuery::VerifyCharacterQuery() :
		Query(),
		station_id(),
		character_id(),
		gold_schema(),
		character_name(),
		approved(),
		scene_id(),
		container_id(),
		x(),
		y(),
		z(),
		containment_flag()
{
}

// ----------------------------------------------------------------------

void TaskVerifyCharacter::addCharacter(StationId suid, const NetworkId &characterId, const TransferCharacterData * transferCharacterData)
{
	LOG("TRACE_LOGIN", ("TaskVerifyCharacter(%d, %s) -- adding", suid, characterId.getValueString().c_str()));

	CharacterRecord *cr = new CharacterRecord;
	cr->m_suid = suid;
	cr->m_characterId = characterId;
	cr->m_transferCharacterData = transferCharacterData;
	m_characters.push_back(cr);
}

// ======================================================================
