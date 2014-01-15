// ======================================================================
//
// ServerShipObjectInterface.h
// asommers
//
// copyright 2004, sony online entertainment
//
// ======================================================================

#ifndef INCLUDED_ServerShipObjectInterface_H
#define INCLUDED_ServerShipObjectInterface_H

// ======================================================================

#include "sharedGame/ShipObjectInterface.h"

class ShipObject;

// ======================================================================

class ServerShipObjectInterface : public ShipObjectInterface
{
public:

	explicit ServerShipObjectInterface(ShipObject const * shipObject);
	virtual ~ServerShipObjectInterface();

	virtual float getMaximumSpeed() const;
	virtual float getSpeedAcceleration() const;
	virtual float getSpeedDeceleration() const;
	virtual float getSlideDampener() const;
	virtual float getMaximumYaw() const;
	virtual float getYawAcceleration() const;
	virtual float getMaximumPitch() const;
	virtual float getPitchAcceleration() const;
	virtual float getMaximumRoll() const;
	virtual float getRollAcceleration() const;

protected:

	ShipObject const * const m_shipObject;

private:

	ServerShipObjectInterface();
	ServerShipObjectInterface(ServerShipObjectInterface const &);
	ServerShipObjectInterface & operator=(ServerShipObjectInterface const &);
};

// ======================================================================

#endif
