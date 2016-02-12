//========================================================================
//
// NpcConversation.cpp
//
// copyright 2001 Sony Online Entertainment
//
//========================================================================

#include "serverGame/FirstServerGame.h"
#include "serverGame/NpcConversation.h"

#include "serverGame/TangibleObject.h"
#include "sharedFoundation/GameControllerMessage.h"
#include "sharedGame/OutOfBandPackager.h"
#include "sharedNetworkMessages/MessageQueueNpcConversationMessage.h"
#include "sharedNetworkMessages/MessageQueueSpatialChat.h"
#include "sharedNetworkMessages/MessageQueueStartNpcConversation.h"
#include "sharedNetworkMessages/MessageQueueStopNpcConversation.h"
#include "sharedNetworkMessages/MessageQueueStringList.h"
#include "sharedObject/Controller.h"

//----------------------------------------------------------------------

namespace NpcConversationNamespace
{
	void sendMessageToClient (const CachedNetworkId & player, int type, MessageQueue::Data & message)
	{
		Object * const playerObject = player.getObject ();		
		Controller * const controller = playerObject ? playerObject->getController () : 0;
		
		if (controller)
		{
			controller->appendMessage(
				type,
				0.0f, 
				&message,
				GameControllerMessageFlags::SEND |
				GameControllerMessageFlags::RELIABLE |
				GameControllerMessageFlags::DEST_AUTH_CLIENT
				);
		}
	}
}

using namespace NpcConversationNamespace;


//----------------------------------------------------------------------


/**
 * Class constructor. Also informs the player a conversation has started.
 *
 * @param player		the player in the conversation
 * @param npc			the npc in the conversation
 */
NpcConversation::NpcConversation(TangibleObject & player, TangibleObject & npc, const std::string & convoName, uint32 appearanceOverrideServerTemplateCrc) :
	m_convoName (convoName),
	m_player    (player),
	m_npc       (npc),
	m_responses (new ResponseVector),
	m_appearanceOverrideSharedTemplateCrc(appearanceOverrideServerTemplateCrc),
	m_finalStringId(),
	m_finalProsePackage()

{
	// tell the player what is going on
	MessageQueueStartNpcConversation * const message = new MessageQueueStartNpcConversation;

	message->setNpc     (m_npc);
	message->setAppearanceOverrideTemplateCrc(m_appearanceOverrideSharedTemplateCrc);

	sendMessageToClient (m_player, CM_npcConversationStart, *message);
}	// NpcConversation::NpcConversation

//----------------------------------------------------------------------

/**
 * Class destructor. Also informs the player the conversation has ended.
 */
NpcConversation::~NpcConversation()
{
	TangibleObject * const player = safe_cast<TangibleObject *>(m_player.getObject());
	if (player != nullptr)
	{
		MessageQueueStopNpcConversation * const message = new MessageQueueStopNpcConversation;

		message->setNpc(m_npc);
		message->setFinalMessageId(m_finalStringId);
		message->setFinalMessageProse(m_finalProsePackage);

		sendMessageToClient(m_player, CM_npcConversationStop, *message);
	}

	delete m_responses;
	m_responses = 0;

}	// NpcConversation::~NpcConversation

//----------------------------------------------------------------------

/**
 * Sends an npc message to the player.
 *
 * @param npcMessage		the message from the npc
 */
void NpcConversation::sendMessage(const Response & npcMessage, const Unicode::String & oob)
{
	// make sure the player and npc are still around
	TangibleObject * const player = safe_cast<TangibleObject *>(m_player.getObject());
	TangibleObject * const npc    = safe_cast<TangibleObject *>(m_npc.getObject());

	if (player == nullptr)
	{
		if (npc != nullptr)
			npc->removeConversation(m_player);
		return;
	}
	if (npc == nullptr)
	{
		player->endNpcConversation();
		return;
	}

	Unicode::String text;
	Unicode::String localOob = oob;

	if (!npcMessage.stringId.isInvalid ())
	{
		ProsePackage pp;
		pp.stringId = npcMessage.stringId;
		OutOfBandPackager::pack (pp, -1, localOob);
	}
	
	if (npcMessage.pp.isValid ())
	{
		OutOfBandPackager::pack (npcMessage.pp, -1, localOob);
	}
	
	const int language = 0;

	const MessageQueueSpatialChat msg (m_npc, m_player, text, 0, 0, 0, MessageQueueSpatialChat::F_skipTarget, language, localOob);
	npc->speakText (msg);

	if (!localOob.empty ())
	{
		text.append (static_cast<size_t>(1), static_cast<unsigned short>(0));
		text.append (localOob);
	}

	//--

	MessageQueueNpcConversationMessage * const message = new MessageQueueNpcConversationMessage;
	message->setNpcMessage (text);
	sendMessageToClient (m_player, CM_npcConversationMessage, *message);

}

//----------------------------------------------------------------------

/**
 * Adds a response to the current response list.
 *
 * @param response		the response to add
 */
void NpcConversation::addResponse(const Response & response)
{
	// make sure the response isn't already in the list
	for (ResponseVector::iterator it = m_responses->begin (); it != m_responses->end (); ++it)
	{
		Response & r = *it;

		if (!response.stringId.isInvalid () && (r.stringId == response.stringId || r.pp.stringId == response.stringId))
		{
			WARNING (true, ("NpcConversation::addResponse tried to add duplicate response stringId=[%s], pp=[%s]", 
				response.stringId.getCanonicalRepresentation ().c_str (), response.pp.stringId.getCanonicalRepresentation ().c_str ()));
			return;
		}
	}

	m_responses->push_back(response);

}	// NpcConversation::addResponse

//----------------------------------------------------------------------

/**
 * Removes a response from the current response list.
 *
 * @param response		the response to remove
 */
void NpcConversation::removeResponse(const Response & response)
{
	for (ResponseVector::iterator it = m_responses->begin (); it != m_responses->end ();)
	{
		Response & r = *it;

		if (!response.stringId.isInvalid () && (r.stringId == response.stringId || r.pp.stringId == response.stringId))
			it = m_responses->erase (it);
		else
			++it;
	}

}	// NpcConversation::removeResponse

//----------------------------------------------------------------------

/**
 * Sends the current responses to the player.
 */
void NpcConversation::sendResponses()
{
	// make sure the player and npc are still around
	TangibleObject * const player = safe_cast<TangibleObject *>(m_player.getObject());
	TangibleObject * const npc = safe_cast<TangibleObject *>(m_npc.getObject());

	const int count = static_cast<int>(m_responses->size());

	if (player == nullptr)
	{
		if (npc != nullptr)
			npc->removeConversation(m_player);
		return;
	}
	if (npc == nullptr)
	{
		player->endNpcConversation();
		return;
	}
	
	MessageQueueStringList * const message = new MessageQueueStringList;

	static Unicode::String text;
	static Unicode::String localOob;

	// convert the string ids to Unicode::Strings
	for (int i = 0; i < count; ++i)
	{
		const Response & response = m_responses->at(i);
	
		text.clear     ();
		localOob.clear ();
		
		if (!response.stringId.isInvalid ())
		{			
			ProsePackage pp;
			pp.stringId = response.stringId;
			OutOfBandPackager::pack (pp, -1, localOob);			
		}
		
		if (response.pp.isValid ())
		{
			OutOfBandPackager::pack (response.pp, -1, localOob);
		}

		if (!localOob.empty ())
		{
			text.append (static_cast<size_t>(1), static_cast<unsigned short>(0));
			text.append (localOob);
		}

		message->addString(text);
	}

	// send the responses to the player
	sendMessageToClient (m_player, CM_npcConversationResponses, *message);

}	// NpcConversation::sendResponses

//----------------------------------------------------------------------

/**
 * Returns the StringId associated with a reponses' index. If the index is out 
 * of range will return the lowest or highest index response. If there are no 
 * responses will return an empty Stringid.
 *
 * @param index		the response index
 *
 * @return the response StringId
 */
const NpcConversation::Response & NpcConversation::getResponse(int index) const
{
	if (m_responses->empty())
		return m_responses->at(static_cast<size_t>(0));

	const int size = static_cast<int>(m_responses->size());
	if (index < 0)
		index = 0;
	if (index >= size)
		index = size - 1;
	return m_responses->at(static_cast<size_t>(index));
}	// NpcConversation::getResponse

//----------------------------------------------------------------------

void NpcConversation::clearResponses()
{
	m_responses->clear();
}

//----------------------------------------------------------------------

int NpcConversation::getResponseCount() const
{
	return m_responses->size();
}

//----------------------------------------------------------------------

void NpcConversation::setFinalStringId(StringId const & stringId)
{
	m_finalStringId = stringId;
}

//----------------------------------------------------------------------

StringId const & NpcConversation::getFinalStringId() const
{
	return m_finalStringId;
}

//----------------------------------------------------------------------

void NpcConversation::setFinalProsePackage(Unicode::String const & pp)
{
	m_finalProsePackage = pp;
}

//----------------------------------------------------------------------

Unicode::String const & NpcConversation::getFinalProsePackage() const
{
	return m_finalProsePackage;
}

//----------------------------------------------------------------------
