// ======================================================================
//
// ShipObjectInterface.h
// asommers
//
// copyright 2004, sony online entertainment
//
// ======================================================================

#ifndef INCLUDED_ShipObjectInterface_H
#define INCLUDED_ShipObjectInterface_H

// ======================================================================

class ShipObjectInterface
{
public:

	virtual float getMaximumSpeed() const = 0;
	virtual float getSpeedAcceleration() const = 0;
	virtual float getSpeedDeceleration() const = 0;
	virtual float getSlideDampener() const = 0;
	virtual float getMaximumYaw() const = 0;
	virtual float getYawAcceleration() const = 0;
	virtual float getMaximumPitch() const = 0;
	virtual float getPitchAcceleration() const = 0;
	virtual float getMaximumRoll() const = 0;
	virtual float getRollAcceleration() const = 0;

protected:

	ShipObjectInterface();
	virtual ~ShipObjectInterface() = 0;
};

// ======================================================================

#endif
