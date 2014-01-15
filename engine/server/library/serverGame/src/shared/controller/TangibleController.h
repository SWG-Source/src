//========================================================================
//
// TangibleController.h
//
// copyright 2001 Sony Online Entertainment
//
//========================================================================

#ifndef	INCLUDED_TangibleController_H
#define	INCLUDED_TangibleController_H

//-----------------------------------------------------------------------

#include "serverGame/ServerController.h"
class AiTurretController;

//----------------------------------------------------------------------

class MessageQueueCommandQueueEnqueue;
class MessageQueueCommandQueueEnqueueFwd;
class TangibleObject;

//-----------------------------------------------------------------------
/**
 * A generic controller for all Tangible Objects.
 */

class TangibleController : public ServerController
{
public:
	explicit TangibleController         (TangibleObject * newOwner);
	~TangibleController                 (void);
	virtual void    setAuthoritative    (bool newAuthoritative);

	void handleCommandQueueEnqueue(MessageQueueCommandQueueEnqueue const * data);
	void handleCommandQueueEnqueueFwd(MessageQueueCommandQueueEnqueueFwd const * data);

	virtual TangibleController * asTangibleController();
	virtual TangibleController const * asTangibleController() const;

	virtual AiTurretController * asAiTurretController();
	virtual AiTurretController const * asAiTurretController() const;

	TangibleObject * getTangibleOwner();
	const TangibleObject * getTangibleOwner() const;

protected:
	virtual void handleMessage (int message, float value, const MessageQueue::Data* data, uint32 flags);

private:

    TangibleController				(void);
    TangibleController				(const TangibleController & other);
    TangibleController&	operator=	(const TangibleController & other);
};

//-----------------------------------------------------------------------

#endif	// INCLUDED_TangibleController_H
