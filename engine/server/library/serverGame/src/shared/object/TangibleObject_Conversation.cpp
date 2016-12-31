//======================================================================
//
// TangibleObject_Conversation.cpp
// copyright(c) 2004 Sony Online Entertainment
//
//======================================================================

#include "serverGame/FirstServerGame.h"
#include "serverGame/TangibleObject_Conversation.h"

#include "serverGame/NpcConversation.h"
#include "serverScript/GameScriptObject.h"
#include "serverScript/ScriptFunctionTable.h"
#include "serverScript/ScriptParameters.h"
#include "sharedFoundation/GameControllerMessage.h"
#include "sharedNetworkMessages/MessageQueueGenericValueType.h"
#include "sharedNetworkMessages/MessageQueueStartNpcConversation.h"
#include "sharedNetworkMessages/MessageQueueStopNpcConversation.h"
#include "sharedObject/Controller.h"
#include "sharedObject/NetworkIdManager.h"
#include "sharedFoundation/NetworkId.h"

//======================================================================

/**
 * Starts up a conversation with an NPC. Note that if a player is starting the
 * conversation it is up to a script to re-call this function via script function
 * npcInitializeConversation().
 *
 * @param npc			the NPC we want to talk to
 * @param convoName		the conversation name
 * @param starter		who started the conversation
 *
 * @return true if the conversation was started, false if not
 */

bool TangibleObject::startNpcConversation(TangibleObject & npc, const std::string & convoName, NpcConversationData::ConversationStarter starter, uint32 const appearanceOverrideSharedTemplateCrc)
{
	if (isAuthoritative())
	{
		// test that I'm a player
		if (!isPlayerControlled())
			return false;

		// test that the npc isn't a player
		if (npc.isPlayerControlled())
			return false;

		// test if already in a conversation
		if (m_npcConversation != nullptr)
			return false;

		if (starter == NpcConversationData::CS_Player)
		{
			// trigger OnStartNpcConversation
			ScriptParams params;
			params.addParam(getNetworkId());
			bool npcHadTrigger = false;
			if (npc.getScriptObject()->hasTrigger(Scripting::TRIG_START_NPC_CONVERSATION))
			{
				IGNORE_RETURN(npc.getScriptObject()->trigAllScripts(Scripting::TRIG_START_NPC_CONVERSATION, params));
				npcHadTrigger = true;
			}

			//inform player that they began a conversation
			if (getScriptObject()->hasTrigger(Scripting::TRIG_START_CONVERSATION))
			{
				ScriptParams playerParams;
				playerParams.addParam(npc.getNetworkId());
				IGNORE_RETURN(getScriptObject()->trigAllScripts(Scripting::TRIG_START_CONVERSATION, playerParams));
			}

			return npcHadTrigger;
		}

		if (convoName.empty())
		{
			DEBUG_WARNING(true,("NPC conversation started with empty name!"));
			return false;
		}

		delete m_npcConversation;
		m_npcConversation = new NpcConversation(*this, npc, convoName, appearanceOverrideSharedTemplateCrc);

		// update any proxies
		addConversation(npc.getNetworkId());
		npc.addConversation(getNetworkId());
	}
	else
	{
		MessageQueueStartNpcConversation * const message = new MessageQueueStartNpcConversation;
		message->setNpc(npc.getNetworkId());
		message->setStarter(starter);
		message->setConversationName(std::string(convoName));
		message->setAppearanceOverrideTemplateCrc(appearanceOverrideSharedTemplateCrc);
		sendControllerMessageToAuthServer(CM_npcConversationStart, message);
	}  //lint !e429 //cust ptr msg
	return true;
}

//-----------------------------------------------------------------------

void TangibleObject::endNpcConversation(StringId const & stringId, Unicode::String const & oob)
{
	if (isAuthoritative() && m_npcConversation)
	{
		m_npcConversation->setFinalStringId(stringId);
		m_npcConversation->setFinalProsePackage(oob);
	}

	endNpcConversation();
}

//-----------------------------------------------------------------------

/**
* Ends a conversation with an NPC.
*/
void TangibleObject::endNpcConversation()
{
	if (isAuthoritative())
	{
		if (m_npcConversation != nullptr)
		{
			// I am a player, end my conversation

			// verify that I am a player
			if (isPlayerControlled())
			{
				TangibleObject * const npc = safe_cast<TangibleObject*>(m_npcConversation->getNPC().getObject());
				if (npc != nullptr)
				{
					// trigger OnEndNpcConversation
					ScriptParams params;
					params.addParam(getNetworkId());
					bool overridden = false;
					if (npc->getScriptObject()->trigAllScripts(Scripting::TRIG_END_NPC_CONVERSATION, params) == SCRIPT_OVERRIDE)
					{
						overridden = true;
					}

					//trigger OnEndNpcConversation
					ScriptParams playerParams;
					playerParams.addParam(npc->getNetworkId());
					if (getScriptObject()->trigAllScripts(Scripting::TRIG_END_CONVERSATION, playerParams) == SCRIPT_OVERRIDE)
					{
						overridden = true;
					}

					if(overridden)
						return;

					// update any proxies
					npc->removeConversation(getNetworkId());
				}
				removeConversation(m_npcConversation->getNPC());
			}
			else
			{
				DEBUG_WARNING(true,("TangibleObject::endNpcConversation: creature %s has a non-nullptr m_npcConversation pointer %p but is not a player-controlled object!", 
					getNetworkId().getValueString().c_str(), m_npcConversation));
				m_conversations.clear();
			}

			delete m_npcConversation;
			m_npcConversation = nullptr;
		}
		else
		{
			// I am an npc, end conversations with players

			// verify that I am an npc
			if (!isPlayerControlled())
			{
				// since m_conversations will be changed as players are forced out
				// of the conversation, we need to make a copy of our conversants
				typedef std::vector<NetworkId> NetworkIdVector;
				NetworkIdVector conversants(m_conversations.get());

				{
					for (NetworkIdVector::iterator it = conversants.begin(); it != conversants.end(); ++it)
					{
						NetworkId const & networkId = *it;
						TangibleObject * const player = dynamic_cast<TangibleObject *>(NetworkIdManager::getObjectById(networkId));
						if (player != nullptr)
							player->endNpcConversation();
					}
				}
			}
			else
			{
				DEBUG_WARNING(true,("TangibleObject::endNpcConversation: creature %s has a nullptr m_npcConversation pointer but is a player-controlled object!", 
					getNetworkId().getValueString().c_str()));
				m_conversations.clear();
			}
		}
	}
	else
	{
		MessageQueueStopNpcConversation * const message = new MessageQueueStopNpcConversation;

		if (m_npcConversation) 
		{
			message->setFinalMessageId(m_npcConversation->getFinalStringId());
			message->setFinalMessageProse(m_npcConversation->getFinalProsePackage());
		}

		sendControllerMessageToAuthServer(CM_npcConversationStop, message);
	}  //lint !e429 //cust ptr msg

}

//-----------------------------------------------------------------------

/**
 * Clears the current responses available to the player.
 */
void TangibleObject::clearNpcConversation()
{
	if (m_npcConversation != nullptr)
	{
		m_npcConversation->clearResponses();
	}
}

//-----------------------------------------------------------------------

/**
 * Sends an npc message to the player.
 *
 * @param message		the message to send
 */
void TangibleObject::sendNpcConversationMessage(const StringId & stringId, const ProsePackage & pp, const Unicode::String & oob)
{
	if (isAuthoritative())
	{
		if (m_npcConversation != nullptr)
		{
			NpcConversation::Response response;
			response.stringId = stringId;
			response.pp       = pp;
			m_npcConversation->sendMessage(response, oob);
		}
	}
	else
	{
		typedef std::pair<std::pair<StringId, ProsePackage>, Unicode::String> Payload;
		sendControllerMessageToAuthServer(CM_forwardNpcConversationMessage, new MessageQueueGenericValueType<Payload>(std::make_pair(std::make_pair(stringId, pp), oob)));
	}
}

//-----------------------------------------------------------------------

/**
 * Adds a response to be available to the player.
 *
 * @param response		the response
 *
 * @return true if the response was added, false if not
 */
bool TangibleObject::addNpcConversationResponse(const StringId & stringId, const ProsePackage & pp)
{
	if (isAuthoritative())
	{
		if (m_npcConversation != nullptr)
		{
			NpcConversation::Response response;
			response.stringId = stringId;
			response.pp       = pp;
			m_npcConversation->addResponse(response);
			return true;
		}
	}
	else
	{
		sendControllerMessageToAuthServer(CM_addNpcConversationResponse, new MessageQueueGenericValueType<std::pair<StringId, ProsePackage> >(std::make_pair(stringId, pp)));
		return true;
	}
	return false;
}

//-----------------------------------------------------------------------

/**
 * Removes a response from the current response list.
 *
 * @param response		the response
 *
 * @return true if the response was removed, false if not
 */
bool TangibleObject::removeNpcConversationResponse(const StringId & stringId, const ProsePackage & pp)
{
	bool result = false;
	if (isAuthoritative())
	{
		if (m_npcConversation != nullptr)
		{
			NpcConversation::Response response;
			response.stringId = stringId;
			response.pp       = pp;
			m_npcConversation->removeResponse(response);
			result = true;
		}
	}
	else
	{
		sendControllerMessageToAuthServer(CM_removeNpcConversationResponse, new MessageQueueGenericValueType<std::pair<StringId, ProsePackage> >(std::make_pair(stringId, pp)));
		result = true;
	}
	return result;
}

//-----------------------------------------------------------------------

/**
 * Sends the current npc conversation responses to the player.
 */
void TangibleObject::sendNpcConversationResponses()
{
	if (isAuthoritative())
	{
		if (m_npcConversation != nullptr)
		{
			m_npcConversation->sendResponses();
		}
	}
	else
	{
		Controller * const controller = NON_NULL(getController());
		controller->appendMessage(CM_sendNpcConversationResponses, 0.0f, nullptr, GameControllerMessageFlags::SEND | GameControllerMessageFlags::RELIABLE | GameControllerMessageFlags::DEST_AUTH_SERVER);
	}
}

//-----------------------------------------------------------------------

/**
 * Called when a player selects a response to an NPC.
 *
 * @param responseIndex		the index of the response chosen(base 0)
 */
void TangibleObject::respondToNpc(int responseIndex)
{
	if (isAuthoritative())
	{
		if (m_npcConversation != nullptr)
		{
			TangibleObject * const npc = safe_cast<TangibleObject *>(m_npcConversation->getNPC().getObject());
			if (npc == nullptr)
			{
				endNpcConversation();
				return;
			}

			if (responseIndex < 0)
				return;

			const NpcConversation::Response & response = m_npcConversation->getResponse(responseIndex);
			if (response.isValid())
				npc->handlePlayerResponseToNpcConversation(m_npcConversation->getName(), getNetworkId(), response.stringId, response.pp);
		}
	}
	else
	{
		Controller * const controller = getController();
		if (controller)
			controller->appendMessage(CM_npcConversationSelect, static_cast<float>(responseIndex), nullptr, GameControllerMessageFlags::SEND | GameControllerMessageFlags::RELIABLE | GameControllerMessageFlags::DEST_AUTH_SERVER);
	}
}

//-----------------------------------------------------------------------

void TangibleObject::handlePlayerResponseToNpcConversation(const std::string & conversationName, const NetworkId & player, const StringId & stringId, const ProsePackage & pp)
{
	if (isAuthoritative())
	{
		TangibleObject * const playerObject = safe_cast<TangibleObject *>(NetworkIdManager::getObjectById(player));
		if (playerObject != nullptr)
		{
			// trigger OnNpcConversationResponse
			ScriptParams params;
			params.addParam(conversationName.c_str());
			params.addParam(player);

			if (!stringId.isInvalid())
				params.addParam(stringId);
			else
				params.addParam(pp.stringId);

			IGNORE_RETURN(getScriptObject()->trigAllScripts(Scripting::TRIG_NPC_CONVERSATION_RESPONSE, params));
			playerObject->sendNpcConversationResponses();
		}
	}
	else
	{
		typedef std::pair<StringId, ProsePackage>       ResponsePair;
		typedef std::pair<NetworkId, ResponsePair>      PlayerResponse;
		typedef std::pair<std::string, PlayerResponse>  Payload;
		typedef MessageQueueGenericValueType<Payload>   MsgType;

		MsgType * const msg = new MsgType(Payload(conversationName, PlayerResponse(player, ResponsePair(stringId, pp))));
		sendControllerMessageToAuthServer(CM_playerResponseToNpcConversation, msg);
	} //lint !e429 //cust ptr msg
}

//-----------------------------------------------------------------------

/**
 * Adds the network id of a creature this creature is talking to.
 *
 * @param conversant		id of the creature we want to add
 */
void TangibleObject::addConversation(const NetworkId & conversant)
{
	// check for duplicate value
	if (m_conversations.find(conversant) >= 0)
		return;

	// add the conversant
	m_conversations.push_back(conversant);
}

//-----------------------------------------------------------------------

/**
 * Removes the network id of a creature this creature is talking to.
 *
 * @param conversant		id of the creature we want to remove
 */
void TangibleObject::removeConversation(const NetworkId & conversant)
{
	// check for duplicate value
	int const index = m_conversations.find(conversant);
	if (index >= 0)
		m_conversations.erase(static_cast<size_t>(index));
}

//======================================================================
