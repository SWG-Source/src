// ======================================================================
//
// Controller.h
// copyright 1998 Bootprint Entertainment
// Copyright 2000-2001 Sony Online Entertainment
// All Rights Reserved.
//
// Abstract controller entity for an object
//
// ======================================================================

#ifndef INCLUDED_Controller_H
#define INCLUDED_Controller_H

// ======================================================================

#include "sharedFoundation/MessageQueue.h"
#include <map>

class CreatureController;
class Object;
class ShipController;
class TangibleController;

// ======================================================================

class Controller
{
public:

	explicit Controller(Object *owner);
	virtual ~Controller();

	float         alter(float time);
	virtual void  conclude();
	
	virtual TangibleController * asTangibleController();
	virtual TangibleController const * asTangibleController() const;

	virtual CreatureController * asCreatureController();
	virtual CreatureController const * asCreatureController() const;

	virtual ShipController * asShipController();
	virtual ShipController const * asShipController() const;

	virtual void  setOwner(Object *newOwner);
	Object       *getOwner();
	const Object *getOwner() const;

                                    // hold message for a little while before sending 
                                    //  only one message per type may be held
	void                        holdMessage( int holdMs, int message, float value, MessageQueue::Data *data, uint32 flags=0);
	MessageQueue::Data*         peekHeldMessage( int message, float *value=0, uint32 *flags=0 );
	void                        sendHeldMessage( int message );
	void                        processHeldMessages( void );

	void          appendMessage(int message, float value, uint32 flags=0);
	void          appendMessage(int message, float value, MessageQueue::Data *data, uint32 flags=0);

	int           getNumberOfMessages() const;
	void          getMessage(int index, int *message, float *value, uint32 *flags=0) const;
	void          getMessage(int index, int *message, float *value, MessageQueue::Data **data, uint32 *flags=0) const;
	void          clearMessageAndData(int i);

	void          setMessageQueueNotification(MessageQueue::Notification *notification);

	MessageQueue * getMessageQueue();

protected:

	struct HeldMessageInfo
	{
		double              sendTime;
		float               value;
                MessageQueue::Data* data;
		uint32              flags;
	};

	virtual float realAlter(float time);

private:

	Controller();
	Controller(const Controller &);
	Controller &operator =(const Controller &);

protected:
	typedef std::map< int, HeldMessageInfo > HeldMessageMap;
	HeldMessageMap   m_heldMessages;   // message type -> held info

	MessageQueue *const m_messageQueue;

private:

	Object       *m_owner;
};

// ======================================================================

inline Object *Controller::getOwner()
{
	return m_owner;
}

// ----------------------------------------------------------------------

inline const Object *Controller::getOwner() const
{
	return m_owner;
}

// ======================================================================

#endif
