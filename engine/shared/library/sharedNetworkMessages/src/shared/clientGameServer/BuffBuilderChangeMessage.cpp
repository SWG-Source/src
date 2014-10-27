// BuffBuilderChangeMessage.cpp
// Copyright 2006, Sony Online Entertainment Inc., all rights reserved. 

//-----------------------------------------------------------------------

#include "sharedNetworkMessages/FirstSharedNetworkMessages.h"
#include "sharedNetworkMessages/BuffBuilderChangeMessage.h"

#include "sharedFoundation/ExitChain.h"
#include "sharedFoundation/GameControllerMessage.h"
#include "sharedFoundation/MemoryBlockManager.h"
#include "sharedNetworkMessages/ControllerMessageFactory.h"

#include <string>

//-----------------------------------------------------------------------

CONTROLLER_MESSAGE_ALLOW_FROM_CLIENT_IMPLEMENTATION(BuffBuilderChangeMessage, CM_buffBuilderChange);

//-----------------------------------------------------------------------

BuffBuilderChangeMessage::BuffBuilderChangeMessage() :
MessageQueue::Data(),
m_bufferId(),
m_recipientId(),
m_startingTime(0),
m_bufferRequiredCredits(0),
m_accepted(false),
m_origin(O_UNKNOWN),
m_buffComponents()
{
}

//-----------------------------------------------------------------------

BuffBuilderChangeMessage & BuffBuilderChangeMessage::operator=(const BuffBuilderChangeMessage & rhs)
{
	if (this == &rhs)
		return *this;

	m_bufferId = rhs.m_bufferId;
	m_recipientId = rhs.m_recipientId;
	m_startingTime = rhs.m_startingTime;
	m_bufferRequiredCredits = rhs.m_bufferRequiredCredits;
	m_accepted = rhs.m_accepted;
	m_origin = rhs.m_origin;
	m_buffComponents = rhs.m_buffComponents;

	return *this;
}

//-----------------------------------------------------------------------

void BuffBuilderChangeMessage::pack(MessageQueue::Data const * const data, Archive::ByteStream & target)
{
	const BuffBuilderChangeMessage* const msg = safe_cast<const BuffBuilderChangeMessage*> (data);

	if (msg)
	{
		Archive::put(target, msg->getBufferId());
		Archive::put(target, msg->getRecipientId());
		Archive::put(target, msg->getStartingTime());
		Archive::put(target, msg->getBufferRequiredCredits());
		Archive::put(target, msg->getAccepted());
		Archive::put(target, msg->getOrigin());

		std::map<std::string, std::pair<int,int> > const & components = msg->getBuffComponents();
		int size = components.size();
		Archive::put(target, size);
		for(std::map<std::string, std::pair<int,int> >::const_iterator j = components.begin(); j != components.end(); ++j)
		{
			Archive::put(target, j->first);
			Archive::put(target, j->second.first);
			Archive::put(target, j->second.second);
		}
	}
}

//-----------------------------------------------------------------------

MessageQueue::Data* BuffBuilderChangeMessage::unpack(Archive::ReadIterator & source)
{
	BuffBuilderChangeMessage * msg = new BuffBuilderChangeMessage;

	NetworkId tempId;
	bool tempBool = false;
	int tempInt = 0;
	time_t tempTime = 0;

	Archive::get(source, tempId);
	msg->setBufferId(tempId);
	Archive::get(source, tempId);
	msg->setRecipientId(tempId);
	Archive::get(source, tempTime);
	msg->setStartingTime(tempTime);
	Archive::get(source, tempInt);
	msg->setBufferRequiredCredits(tempInt);
	Archive::get(source, tempBool);
	msg->setAccepted(tempBool);
	Archive::get(source, tempInt);
	Origin const origin = static_cast<Origin>(tempInt);
	msg->setOrigin(origin);

	int size = 0;
	Archive::get(source, size);
	std::string name;
	int value = 0;
	int expertiseModifier = 0;
	for(int j = 0; j < size; ++j)
	{
		Archive::get(source, name);
		Archive::get(source, value); // count
		Archive::get(source, expertiseModifier);
		for(int k = 0; k < value; ++k)
		{
			msg->addBuffComponent(name, expertiseModifier); 
		}
	}

	return msg;
}

//-----------------------------------------------------------------------

void BuffBuilderChangeMessage::addBuffComponent(std::string const & name, int expertiseModifier)
{
	int value = 1;
	if(m_buffComponents.find(name) != m_buffComponents.end())
	{
		value = m_buffComponents[name].first + 1;
	}

	m_buffComponents[name].first = value;
	m_buffComponents[name].second = expertiseModifier;
}

//-----------------------------------------------------------------------

void BuffBuilderChangeMessage::setBuffComponents(std::map<std::string, std::pair<int,int> > const & buffComponents)
{
	m_buffComponents = buffComponents;
}


//======================================================================
