//========================================================================
//
// VehicleController.h
//
// copyright 2001 Sony Online Entertainment
//
//========================================================================

#ifndef	INCLUDED_VehicleController_H
#define	INCLUDED_VehicleController_H

//-----------------------------------------------------------------------

#include "serverGame/TangibleController.h"

//-----------------------------------------------------------------------

class VehicleObject;

//-----------------------------------------------------------------------

class VehicleController : public TangibleController
{
public:
	explicit		VehicleController		(VehicleObject * newOwner);
					~VehicleController		();

private:
	VehicleController            (void);
	VehicleController            (const VehicleController & other);
	VehicleController& operator=  (const VehicleController & other);
};

//-----------------------------------------------------------------------

#endif	// INCLUDED_VehicleController_H
