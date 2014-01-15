// ======================================================================
//
// ServerController.h
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_ServerController_H
#define INCLUDED_ServerController_H

// ======================================================================

#include "serverGame/ServerObject.h"
#include "sharedFoundation/Watcher.h"
#include "sharedObject/NetworkController.h"
#include "sharedMath/Transform.h"

//-----------------------------------------------------------------------

class CellProperty;

//----------------------------------------------------------------------

/**
 * The base class for server-side controllers.
 *
 */
class ServerController : public NetworkController
{
public:
	explicit                ServerController	(Object * newOwner);
	virtual                 ~ServerController	(void);

	virtual void            conclude            ();
	virtual void            onClientReady       ();
	virtual void            onClientLost        ();

	virtual void            sendControllerMessage(const ObjControllerMessage& msg);

	virtual const Vector &  getCurrentVelocity() const;
	virtual void            setAuthoritative	(bool newAuthoritative);
	virtual void            setOwner(Object *newOwner);
	
	virtual void            endBaselines();
	
	virtual int             elevatorMove (int nFloors);
	virtual void            teleport(Transform const &goal, ServerObject *goalObj);


	/** object modification functions that should only be called on authoritative
	 *  objects
	 */
	void                   setObjectName(const Unicode::String &name);
	ServerObject *         getServerOwner();
	const ServerObject *   getServerOwner() const;

	virtual void           onAppendMessage();
	virtual void           handleMessage (int message, float value, const MessageQueue::Data* data, uint32 flags);
	virtual ServerObject const * getGoalCellObject () const;

	int getMoveSequenceNumber() const;

protected:

	virtual float           realAlter          (float time);
	virtual void            onMoveFailed       ();
	virtual void            handleNetUpdateTransform (const MessageQueueDataTransform& message);
	virtual void            handleNetUpdateTransformWithParent (const MessageQueueDataTransformWithParent& message);

	virtual void            setCurrentVelocity (const Vector & newVelocity);
	virtual void            setGoal            (Transform const & newGoal, ServerObject * goalCellObject, bool teleport = false );
	virtual void            moveToGoal         ();

	virtual bool            setObjectCell      (ServerObject * newCellObject);

	virtual Transform const &    getGoalTransform  () const;

	virtual void synchronizeTransform();

protected:
	bool       m_sendReliableTransformThisFrame;
	int        m_moveSequenceNumber;

private:
	ServerController ();
	ServerController (const ServerController & other);
	ServerController&	operator= (const ServerController & rhs);

	void updateTransform();

private:

	class MessageQueueNotification: public MessageQueue::Notification
	{
	public:
		MessageQueueNotification();
		virtual ~MessageQueueNotification();

		void setController(ServerController *serverController);
		virtual void onChanged() const;

	private:
		MessageQueueNotification(MessageQueueNotification const &);
		MessageQueueNotification& operator=(MessageQueueNotification const &);

	private:
		ServerController *m_serverController;
	};

	Vector      m_currentVelocity;
	Vector      m_oldVelocity;


	Transform       m_goalTransform;        // where the client wants to move the server object to
	Watcher<ServerObject> m_goalCellObject;
	bool            m_bProcessingMessages;  // true while the controller is processing its message queue
	bool            m_bHasGoal;
	bool            m_bAtGoal;
	bool            m_bTeleport;
	bool            m_alteredSinceConclude;

	MessageQueueNotification m_notification;
};

// ----------------------------------------------------------------------

inline const Vector & ServerController::getCurrentVelocity() const
{
	return m_currentVelocity;
}

//-----------------------------------------------------------------------

inline void ServerController::setCurrentVelocity(const Vector& v)
{
	m_currentVelocity = v;
}


//-----------------------------------------------------------------------

inline const ServerObject * ServerController::getServerOwner() const
{
	return safe_cast<const ServerObject *>(getOwner());
}

// ----------------------------------------------------------------------

inline ServerObject * ServerController::getServerOwner()
{
	return safe_cast<ServerObject *>(getOwner());
}

// ----------------------------------------------------------------------

inline Transform const & ServerController::getGoalTransform() const
{
	return m_goalTransform;
}

// ----------------------------------------------------------------------

inline ServerObject const * ServerController::getGoalCellObject() const
{
	return m_goalCellObject;
}


// ======================================================================


#endif	//_ServerController_H


