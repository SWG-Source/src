//========================================================================
//
// UniverseController.h
//
// copyright 2001 Sony Online Entertainment
//
//========================================================================

#ifndef	INCLUDED_UniverseController_H
#define	INCLUDED_UniverseController_H

//-----------------------------------------------------------------------

#include "serverGame/ServerController.h"
#include "serverGame/UniverseObject.h"

//----------------------------------------------------------------------

//class UniverseObject;

//-----------------------------------------------------------------------
/**
 * A generic controller for all Universe Objects.
 */

class UniverseController : public ServerController
{
  public:
    explicit UniverseController         (UniverseObject * newOwner);
    ~UniverseController                 (void);

  private:
    UniverseController				(void);
    UniverseController				(const UniverseController & other);
    UniverseController&	operator=	(const UniverseController & other);
};

//-----------------------------------------------------------------------

#endif	// INCLUDED_UniverseController_H
