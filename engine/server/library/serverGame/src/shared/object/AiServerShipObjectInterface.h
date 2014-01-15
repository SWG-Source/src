// ======================================================================
//
// AiServerShipObjectInterface.h
//
// copyright 2004, sony online entertainment
//
// ======================================================================

#ifndef INCLUDED_AiServerShipObjectInterface_H
#define INCLUDED_AiServerShipObjectInterface_H

// ======================================================================

#include "serverGame/ServerShipObjectInterface.h"

struct AiShipPilotData;
class ShipObject;

// ======================================================================

class AiServerShipObjectInterface : public ServerShipObjectInterface
{
public:

	static void install();
	static void setFastAxisEnabled(bool const enabled);
	static bool isFastAxisEnabled();

public:

	AiServerShipObjectInterface(ShipObject const * shipObject, AiShipPilotData const & aiShipPilotData, bool const modifySpeed);

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

private:

	bool const m_modifySpeed;
	AiShipPilotData const & m_aiShipPilotData;

	AiServerShipObjectInterface();
	AiServerShipObjectInterface(AiServerShipObjectInterface const &);
	AiServerShipObjectInterface & operator=(AiServerShipObjectInterface const &);
};

// ======================================================================

#endif // INCLUDED_AiServerShipObjectInterface_H
