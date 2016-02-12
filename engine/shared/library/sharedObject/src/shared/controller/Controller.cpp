// ======================================================================
//
// Controller.cpp
// copyright 1998 Bootprint Entertainment
// Copyright 2000-2001 Sony Online Entertainment
// All Rights Reserved.
//
// ======================================================================

#include "sharedObject/FirstSharedObject.h"
#include "sharedObject/Controller.h"

#include "sharedObject/AlterResult.h"
#include "sharedObject/Object.h"
#include "sharedFoundation/Clock.h"

// ======================================================================

Controller::Controller(Object *const owner) :
	m_messageQueue(new MessageQueue(16)),
	m_owner(owner)
{
}

// ----------------------------------------------------------------------

Controller::~Controller()
{
	HeldMessageMap::iterator iter;
	for ( iter = m_heldMessages.begin(); iter != m_heldMessages.end(); ++iter )
	{
		delete( iter->second.data );
	}
	m_heldMessages.clear();


	delete m_messageQueue;

	m_owner = 0;
}

// ----------------------------------------------------------------------

float Controller::alter(float time)
{
	NOT_NULL(m_messageQueue);
	m_messageQueue->beginFrame();

	processHeldMessages();

	return realAlter(time);
}

// ----------------------------------------------------------------------

void Controller::conclude()
{
}

// ----------------------------------------------------------------------

void Controller::setOwner(Object *const owner)
{
	m_owner = owner;
}


//
// Hold onto this message for a while before sending
//
void Controller::holdMessage( int holdMs, int message, float value, MessageQueue::Data *data, uint32 flags)
{
	if ( m_heldMessages.count(message) )  // only one per type
		sendHeldMessage( message );
	m_heldMessages[ message ].sendTime = Clock::getCurrentTime() + (double(holdMs) / 1000.0);
	m_heldMessages[ message ].value = value;
	m_heldMessages[ message ].data = data;
	m_heldMessages[ message ].flags = flags;

	if (m_owner && m_owner->isInitialized())
		m_owner->scheduleForAlter();
}

//
// Check out the current held message
//
MessageQueue::Data* Controller::peekHeldMessage( int message, float *value, uint32 *flags )
{
	HeldMessageMap::iterator iter = m_heldMessages.find( message );
	if ( iter == m_heldMessages.end() )
		return nullptr;
	if ( value )
		*value = iter->second.value;
	if ( flags )
		*flags = iter->second.flags;
	return iter->second.data;
}

//
// Cause the held message of this type to get sent now
//
void Controller::sendHeldMessage( int message )
{
	HeldMessageMap::iterator iter = m_heldMessages.find( message );
	if ( iter == m_heldMessages.end() )
		return;
	appendMessage( message, iter->second.value, iter->second.data, iter->second.flags );
	m_heldMessages.erase( iter );
}

//
// Cause the held message of this type to get sent if the hold time has expired 
//
void Controller::processHeldMessages( void )
{
	double now = Clock::getCurrentTime();
	HeldMessageMap::iterator iter;
	for ( iter = m_heldMessages.begin(); iter != m_heldMessages.end(); )
	{
		if ( iter->second.sendTime <= now )
		{
			appendMessage( iter->first, iter->second.value, iter->second.data, iter->second.flags );
			m_heldMessages.erase( iter++ );
		}
		else
		{
			++iter;
		}
	}
	if ( ! m_heldMessages.empty() )
	{
		if (m_owner && m_owner->isInitialized())
			m_owner->scheduleForAlter();
	}
}



// ----------------------------------------------------------------------

void Controller::appendMessage(const int message, const float value, const uint32 flags)
{
	NOT_NULL(m_messageQueue);
	m_messageQueue->appendMessage(message, value, flags);

	if (message==0)
	{
		WARNING(true, ("trying to append message 0"));
	}
	
	//-- We must guarantee that the owner object gets an alter to process this message.
	// @todo resolve: if this object is handled via object lists (e.g. a UI object), this 
	//       should not go on the alter scheduler.
	if (m_owner && m_owner->isInitialized())
		m_owner->scheduleForAlter();
}

// ----------------------------------------------------------------------

void Controller::appendMessage(const int message, const float value, MessageQueue::Data *const data, const uint32 flags)
{
	NOT_NULL(m_messageQueue);
	m_messageQueue->appendMessage(message, value, data, flags);

	//-- We must guarantee that the owner object gets an alter to process this message.
	// @todo resolve: if this object is handled via object lists (e.g. a UI object), this 
	//       should not go on the alter scheduler.
	if (m_owner && m_owner->isInitialized())
		m_owner->scheduleForAlter();
}

// ----------------------------------------------------------------------

int Controller::getNumberOfMessages() const
{
	return m_messageQueue->getNumberOfMessages();
}

// ----------------------------------------------------------------------

void Controller::getMessage(int index, int *message, float *value, uint32 *flags) const
{
	m_messageQueue->getMessage(index, message, value, flags);
}

// ----------------------------------------------------------------------

void Controller::getMessage(int index, int *message, float *value, MessageQueue::Data **data, uint32 *flags) const
{
	m_messageQueue->getMessage(index, message, value, data, flags);
}

// ----------------------------------------------------------------------

void Controller::clearMessageAndData(int index)
{
	m_messageQueue->clearMessageData(index);
	m_messageQueue->clearMessage(index);
}

// ----------------------------------------------------------------------

void Controller::setMessageQueueNotification(MessageQueue::Notification *notification)
{
	m_messageQueue->setNotification(notification);
}

//----------------------------------------------------------------------

MessageQueue * Controller::getMessageQueue()
{
	return m_messageQueue;
}

//----------------------------------------------------------------------

TangibleController * Controller::asTangibleController()
{
	return nullptr;
}

//----------------------------------------------------------------------

TangibleController const * Controller::asTangibleController() const
{
	return nullptr;
}

//----------------------------------------------------------------------

CreatureController * Controller::asCreatureController()
{
	return nullptr;
}

//----------------------------------------------------------------------

CreatureController const * Controller::asCreatureController() const
{
	return nullptr;
}

//----------------------------------------------------------------------

ShipController * Controller::asShipController()
{
	return nullptr;
}

//----------------------------------------------------------------------

ShipController const * Controller::asShipController() const
{
	return nullptr;
}

// ======================================================================
// PROTECTED Controller
// ======================================================================

float Controller::realAlter(float /*time*/)
{
	// Indicate that no alter is required for this class' functionality.
	return AlterResult::cms_keepNoAlter;
}

// ======================================================================
