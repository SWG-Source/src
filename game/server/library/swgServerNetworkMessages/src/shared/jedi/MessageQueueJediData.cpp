// ======================================================================
//
// MessageQueueJediData.cpp
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#include "swgServerNetworkMessages/FirstSwgServerNetworkMessages.h"
#include "swgServerNetworkMessages/MessageQueueJediData.h"

#include <string>
#include "sharedFoundation/MemoryBlockManager.h"
#include "sharedFoundation/ExitChain.h"
#include "sharedFoundation/GameControllerMessage.h"
#include "sharedMathArchive/VectorArchive.h"
#include "sharedNetworkMessages/ControllerMessageFactory.h"

//===================================================================

CONTROLLER_MESSAGE_IMPLEMENTATION(MessageQueueJediData, CM_addJedi);

//===================================================================


// ----------------------------------------------------------------------

MessageQueueJediData::MessageQueueJediData(const NetworkId & id, 
	const Unicode::String & name, const Vector & location, const std::string & scene, 
	int visibility, int bountyValue, int level, int hoursAlive, int state, int spentJediSkillPoints, int faction) :
	m_id(id),
	m_name(name),
	m_location(location),
	m_scene(scene),
	m_visibility(visibility),
	m_bountyValue(bountyValue),
	m_level(level),
	m_hoursAlive(hoursAlive),
	m_state(state),
	m_spentJediSkillPoints(spentJediSkillPoints),
	m_faction(faction)
{
}	

// ----------------------------------------------------------------------

MessageQueueJediData::~MessageQueueJediData()
{
	
}

void MessageQueueJediData::pack(const MessageQueue::Data* const data, Archive::ByteStream & target)
{
	const MessageQueueJediData* const msg = safe_cast<const MessageQueueJediData*> (data);
	if (msg)
	{
		Archive::put(target, msg->m_id);
		Archive::put(target, msg->m_name);
		Archive::put(target, msg->m_location);
		Archive::put(target, msg->m_scene);
		Archive::put(target, msg->m_visibility);
		Archive::put(target, msg->m_bountyValue);
		Archive::put(target, msg->m_level);
		Archive::put(target, msg->m_hoursAlive);
		Archive::put(target, msg->m_state);
		Archive::put(target, msg->m_spentJediSkillPoints);
		Archive::put(target, msg->m_faction);
	}
}

// ----------------------------------------------------------------------

MessageQueue::Data* MessageQueueJediData::unpack(Archive::ReadIterator & source)
{
	NetworkId id;
	Unicode::String name;
	Vector location;
	std::string scene;
	int visibility, bountyValue, level, hoursAlive, state, spentJediSkillPoints, faction;

	Archive::get(source, id);
	Archive::get(source, name);
	Archive::get(source, location);
	Archive::get(source, scene);
	Archive::get(source, visibility);
	Archive::get(source, bountyValue);
	Archive::get(source, level);
	Archive::get(source, hoursAlive);
	Archive::get(source, state);
	Archive::get(source, spentJediSkillPoints);
	Archive::get(source, faction);
	
	MessageQueueJediData * result = new MessageQueueJediData(id, name, location, scene, visibility, bountyValue, level, hoursAlive, state, spentJediSkillPoints, faction);
	return result;
}


// ======================================================================
