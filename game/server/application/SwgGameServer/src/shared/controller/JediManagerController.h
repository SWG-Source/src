//========================================================================
//
// JediManagerController.h
//
// copyright 2001 Sony Online Entertainment
//
//========================================================================

#ifndef	INCLUDED_JediManagerController_H
#define	INCLUDED_JediManagerController_H

#include "serverGame/UniverseController.h"

class JediManagerObject;


//----------------------------------------------------------------------


/**
 * A generic controller for all JediManager Objects.
 */
class JediManagerController : public UniverseController
{
public:
	explicit JediManagerController (JediManagerObject * newOwner);
	virtual ~JediManagerController (void);
	
protected:
	virtual void handleMessage (int message, float value, const MessageQueue::Data* data, uint32 flags);

private:
    JediManagerController				(void);
    JediManagerController				(const JediManagerController & other);
    JediManagerController&	operator=	(const JediManagerController & other);
};


//-----------------------------------------------------------------------


#endif	// INCLUDED_JediManagerController_H
