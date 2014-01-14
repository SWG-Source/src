// ======================================================================
//
// NetworkController.h
// copyright 2000 Verant Interactive
//
// ======================================================================

#ifndef	INCLUDED_NetworkController_H
#define	INCLUDED_NetworkController_H

// ======================================================================

#include "sharedObject/Controller.h"

class Baselines;
class GameNetworkMessage;
class MessageQueueDataTransform;
class MessageQueueDataTransformWithParent;
class MessageQueueSetMemberData;
class ObjControllerMessage;

// ======================================================================

class NetworkController : public Controller
{
public:

	explicit NetworkController(Object* newOwner);
	virtual  ~NetworkController();

	/**
		NetworkController::conclude() gathers all messages in it's
		message queue and sends those with the ControllerMessageFlag::SEND
		option set.
	*/
	virtual void            conclude ();

	virtual void            sendControllerMessage(const ObjControllerMessage& msg) = 0;

	bool                    getAuthoritative  () const;
	virtual void            setAuthoritative (bool newAuthoritative);

protected:

	virtual void            handleMessage (int message, float value, const MessageQueue::Data* data, uint32 flags);
	virtual void            handleNetUpdateTransform (const MessageQueueDataTransform& message) = 0;
	virtual void            handleNetUpdateTransformWithParent (const MessageQueueDataTransformWithParent& message) = 0;

private:

	NetworkController();
	NetworkController(const NetworkController & other);
	NetworkController& operator=(const NetworkController & rhs);
};

// ======================================================================

#endif
