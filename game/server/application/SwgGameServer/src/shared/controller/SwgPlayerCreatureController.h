//========================================================================
//
// SwgPlayerCreatureController.h
//
// copyright 2001 Sony Online Entertainment
//
//========================================================================

#ifndef	INCLUDED_SwgPlayerCreatureController_H
#define	INCLUDED_SwgPlayerCreatureController_H

//-----------------------------------------------------------------------

#include "serverGame/PlayerCreatureController.h"


//-----------------------------------------------------------------------

class SwgCreatureObject;


//-----------------------------------------------------------------------

class SwgPlayerCreatureController : public PlayerCreatureController
{
public:
	explicit SwgPlayerCreatureController (SwgCreatureObject * newOwner);
	         ~SwgPlayerCreatureController ();

protected:
	virtual void handleMessage (int message, float value, const MessageQueue::Data* data, uint32 flags);

private:
	// Disabled.
	SwgPlayerCreatureController            (void);
	SwgPlayerCreatureController            (const SwgPlayerCreatureController & other);
	SwgPlayerCreatureController& operator= (const SwgPlayerCreatureController & other);
};

//--------------------------------------------------------------------

#endif	// INCLUDED_SwgPlayerCreatureController_H

