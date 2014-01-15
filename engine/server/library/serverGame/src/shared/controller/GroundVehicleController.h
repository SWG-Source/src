// GroundVehicleController.h
// copyright 2000 Verant Interactive
// Author: Justin Randall

#ifndef	_GroundVehicleController_H
#define	_GroundVehicleController_H

//-----------------------------------------------------------------------

#include "serverGame/VehicleController.h"
#include "sharedMath/Vector.h"

//-----------------------------------------------------------------------

class VehicleObject;

//-----------------------------------------------------------------------

class GroundVehicleController : public VehicleController
{
public:
	explicit		GroundVehicleController		(VehicleObject * newOwner);
					~GroundVehicleController		();

protected:
	Vector          yawPitchRollRates;
	real            maxForwardVelocity;

private:
	GroundVehicleController						(void);
	GroundVehicleController						(const GroundVehicleController & other);
	GroundVehicleController&	operator=			(const GroundVehicleController & other);
};

//-----------------------------------------------------------------------

#endif	// _GroundVehicleController_H
