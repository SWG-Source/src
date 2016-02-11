// ============================================================================
//
// ServerBuffBuilderManager.cpp
// Copyright 2006 Sony Online Entertainment, Inc.
//
// ============================================================================

#include "serverGame/FirstServerGame.h"
#include "serverGame/ServerBuffBuilderManager.h"

#include "serverGame/CreatureObject.h"
#include "sharedFoundation/ExitChain.h"
#include "sharedMessageDispatch/Transceiver.h"
#include "sharedObject/Controller.h"
#include "sharedObject/NetworkIdManager.h"
#include "sharedObject/Object.h"

#include "serverScript/GameScriptObject.h"
#include "serverScript/ScriptFunctionTable.h"
#include "serverScript/ScriptParameters.h"

// ============================================================================

namespace ServerBuffBuilderManagerNamespace
{
	class MyCallback : public MessageDispatch::Callback
	{
	public:
		MyCallback ()
		: MessageDispatch::Callback ()
		{
			
		}
	};


	bool ms_installed = false;
	MyCallback * ms_callback = 0;
}

using namespace ServerBuffBuilderManagerNamespace;

// ============================================================================

void ServerBuffBuilderManager::install()
{
	DEBUG_FATAL(ms_installed, ("ServerImageDesignerManager::install Already Installed"));
	ms_installed = true;
	ms_callback = new MyCallback();
	ExitChain::add(ServerBuffBuilderManager::remove, "ServerBuffBuilderManager::remove", 0, false);
}

//-----------------------------------------------------------------------------

void ServerBuffBuilderManager::remove()
{
	DEBUG_FATAL(!ms_installed, ("ServerBuffBuilderManager::remove - Not Installed"));
	ms_installed = false;

	delete ms_callback;
	ms_callback = nullptr;
}

//-----------------------------------------------------------------------------

bool ServerBuffBuilderManager::makeChanges(SharedBuffBuilderManager::Session const & session)
{
	if(!ms_installed)
	{
		DEBUG_FATAL(true, ("ServerBuffBuilderManager not installed"));
		return false;
	}

	NetworkId const & recipientId = session.recipientId;
	Object * const recipientObj = NetworkIdManager::getObjectById(recipientId);
	ServerObject * const recipientServerObj = recipientObj ? recipientObj->asServerObject() : nullptr;
	CreatureObject * const recipient = recipientServerObj ? recipientServerObj->asCreatureObject() : nullptr;
	NetworkId const & bufferId = session.bufferId;
	Object * const bufferObj = NetworkIdManager::getObjectById(bufferId);
	ServerObject * const bufferServerObj = bufferObj ? bufferObj->asServerObject() : nullptr;
	CreatureObject * const buffer = bufferServerObj ? bufferServerObj->asCreatureObject() : nullptr;
	if(buffer && recipient)
	{
		sendSessionToScript(session, session.bufferId, static_cast<int>(Scripting::TRIG_BUFF_BUILDER_COMPLETED));
	}
	return true;
}

//----------------------------------------------------------------------

void ServerBuffBuilderManager::sendSessionToScriptForValidation(SharedBuffBuilderManager::Session const & session)
{
	NetworkId const & bufferId = session.bufferId;
	NetworkId const & recipientId = session.recipientId;
	Object const * const bufferObj = NetworkIdManager::getObjectById(bufferId);
	ServerObject const * const bufferServer = bufferObj ? bufferObj->asServerObject() : nullptr;
	CreatureObject const * const buffer = bufferServer ? bufferServer->asCreatureObject() : nullptr;
	Object * const recipientObj = NetworkIdManager::getObjectById(recipientId);
	ServerObject * const recipientServer = recipientObj ? recipientObj->asServerObject() : nullptr;
	CreatureObject * const recipient = recipientServer ? recipientServer->asCreatureObject() : nullptr;
	if(buffer && recipient)
	{
		sendSessionToScript(session, session.bufferId, static_cast<int>(Scripting::TRIG_BUFF_BUILDER_VALIDATE));
	}
}

//----------------------------------------------------------------------

void ServerBuffBuilderManager::sendSessionToScript(SharedBuffBuilderManager::Session const & session, NetworkId const & objectToTriggerId, int const trigger)
{
	Object * const objectToTrigger = NetworkIdManager::getObjectById(objectToTriggerId);
	ServerObject * const serverObjectToTrigger = objectToTrigger ? objectToTrigger->asServerObject() : nullptr;
	if(serverObjectToTrigger)
	{
		GameScriptObject * const scriptObject = serverObjectToTrigger->getScriptObject();
		if (scriptObject)
		{
			ScriptParams params;
			params.addParam(session.bufferId, "bufferId");
			params.addParam(session.recipientId, "recipientId");
			params.addParam(static_cast<int>(session.startingTime), "startingTime");
			params.addParam(session.bufferRequiredCredits, "bufferRequiredCredits");
			params.addParam(session.bufferRequiredCredits, "recipientPaidCredits");
			params.addParam(session.accepted, "accepted");
			std::vector<const char *> buffComponentKeys;
			std::vector<int> buffComponentValues;
			
			for(std::map<std::string, std::pair<int,int> >::const_iterator j = session.buffComponents.begin(); j != session.buffComponents.end(); ++j)
			{
				buffComponentKeys.push_back(j->first.c_str());
				buffComponentValues.push_back(j->second.first);
			}
			params.addParam(buffComponentKeys, "buffComponentKeys");
			params.addParam(buffComponentValues, "buffComponentValues");
			
			IGNORE_RETURN(scriptObject->trigAllScripts(static_cast<Scripting::TrigId>(trigger), params));
		}
	}
}

//----------------------------------------------------------------------

void ServerBuffBuilderManager::cancelSession(NetworkId const & bufferId, NetworkId const & recipientId)
{
	//send the cancel message to the buffer
	Object * const bufferObject = NetworkIdManager::getObjectById(bufferId);
	Controller * const bufferController = bufferObject ? bufferObject->getController() : nullptr;
	if(bufferController)
	{
		BuffBuilderChangeMessage * outMsg = new BuffBuilderChangeMessage();

		outMsg->setBufferId(bufferId);
		outMsg->setRecipientId(recipientId);
		outMsg->setOrigin(BuffBuilderChangeMessage::O_SERVER);
		bufferController->appendMessage(static_cast<int>(CM_buffBuilderCancel), 0.0f, outMsg, GameControllerMessageFlags::SEND | GameControllerMessageFlags::RELIABLE | GameControllerMessageFlags::DEST_AUTH_CLIENT);
	}

	//send the cancel message to the recipient
	Object * const recipientObject = NetworkIdManager::getObjectById(recipientId);
	Controller * const recipientController = recipientObject ? recipientObject->getController() : nullptr;
	if(recipientController && bufferController != recipientController)
	{
		BuffBuilderChangeMessage * outMsg = new BuffBuilderChangeMessage();

		outMsg->setBufferId(bufferId);
		outMsg->setRecipientId(recipientId);
		outMsg->setOrigin(BuffBuilderChangeMessage::O_SERVER);
		recipientController->appendMessage(static_cast<int>(CM_buffBuilderCancel), 0.0f, outMsg, GameControllerMessageFlags::SEND | GameControllerMessageFlags::RELIABLE | GameControllerMessageFlags::DEST_AUTH_CLIENT);
	}

	//send cancel trigger to buffer player
	ServerObject * const bufferServerObject = bufferObject ? bufferObject->asServerObject() : nullptr;
	if(bufferServerObject)
	{
		GameScriptObject * const scriptObject = bufferServerObject->getScriptObject();
		if (scriptObject)
		{
			ScriptParams params;
			IGNORE_RETURN(scriptObject->trigAllScripts(Scripting::TRIG_BUFF_BUILDER_CANCELED, params));
		}
	}

	//send cancel trigger to recipient player
	ServerObject * const recipientServerObject = recipientObject ? recipientObject->asServerObject() : nullptr;
	if(recipientServerObject && recipientServerObject != bufferServerObject)
	{
		GameScriptObject * const scriptObject = recipientServerObject->getScriptObject();
		if (scriptObject)
		{
			ScriptParams params;
			IGNORE_RETURN(scriptObject->trigAllScripts(Scripting::TRIG_BUFF_BUILDER_CANCELED, params));
		}
	}
	SharedBuffBuilderManager::endSession(bufferId);
}

// ============================================================================




