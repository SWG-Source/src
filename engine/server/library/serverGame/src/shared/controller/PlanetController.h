//========================================================================
//
// PlanetController.h
//
// copyright 2001 Sony Online Entertainment
//
//========================================================================

#ifndef	INCLUDED_PlanetController_H
#define	INCLUDED_PlanetController_H

//-----------------------------------------------------------------------

#include "Archive/AutoDeltaByteStream.h"
#include "serverGame/PlanetObject.h" 
#include "serverGame/UniverseController.h"

//-----------------------------------------------------------------------

class PlanetController : public UniverseController
{
public:
	explicit		PlanetController	(PlanetObject * newOwner);
					~PlanetController	();

protected:
	virtual void  handleMessage  (int message, float value, const MessageQueue::Data* data, uint32 flags);

private:
	PlanetController					(void);
	PlanetController					(const PlanetController & other);
	PlanetController&	operator=		(const PlanetController & other);
};

//-----------------------------------------------------------------------

#endif	// INCLUDED_PlanetController_H
