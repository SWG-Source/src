// ======================================================================
//
// AiServerShipObjectInterface.cpp
//
// copyright 2004, sony online entertainment
//
// ======================================================================

#include "serverGame/FirstServerGame.h"
#include "serverGame/AiServerShipObjectInterface.h"

#include "serverGame/AiShipPilotData.h"
#include "serverGame/ShipObject.h"
#include "sharedFoundation/ConfigFile.h"

// ======================================================================
//
// AiServerShipObjectInterfaceNamespace
//
// ======================================================================

namespace AiServerShipObjectInterfaceNamespace
{
	bool s_fastAxis = true;
}

using namespace AiServerShipObjectInterfaceNamespace;

// ======================================================================
//
// AiServerShipObjectInterface
//
// ======================================================================

// ----------------------------------------------------------------------

void AiServerShipObjectInterface::install()
{
	setFastAxisEnabled(ConfigFile::getKeyBool("GameServer", "spaceAiFastAxisEnabled", true));
}

// ----------------------------------------------------------------------

AiServerShipObjectInterface::AiServerShipObjectInterface(ShipObject const * const shipObject, AiShipPilotData const & aiShipPilotData, bool const modifySpeed)
 : ServerShipObjectInterface(shipObject)
 , m_modifySpeed(modifySpeed)
 , m_aiShipPilotData(aiShipPilotData)
{
}

// ----------------------------------------------------------------------

float AiServerShipObjectInterface::getMaximumSpeed() const
{
	float const maximumSpeed = ServerShipObjectInterface::getMaximumSpeed();

	if (!m_modifySpeed)
	{
		return (maximumSpeed * m_aiShipPilotData.m_nonCombatMaxSpeedPercent);
	}

	return maximumSpeed;
}

// ----------------------------------------------------------------------

float AiServerShipObjectInterface::getSpeedAcceleration() const
{
	float const speedAcceleration = ServerShipObjectInterface::getSpeedAcceleration();

	if (!m_modifySpeed)
	{
		float const maximumSpeed = getMaximumSpeed();

		return std::max(speedAcceleration, maximumSpeed * 2.0f);
	}

	return speedAcceleration;
}

// ----------------------------------------------------------------------

float AiServerShipObjectInterface::getSpeedDeceleration() const
{
	float const speedDeceleration = ServerShipObjectInterface::getSpeedDeceleration();

	if (!m_modifySpeed)
	{
		float const maximumSpeed = getMaximumSpeed();

		return std::max(speedDeceleration, maximumSpeed * 2.0f);
	}

	return speedDeceleration;
}

// ----------------------------------------------------------------------

float AiServerShipObjectInterface::getSlideDampener() const
{
	float const slideDampener = ServerShipObjectInterface::getSlideDampener();

	return slideDampener;
}

// ----------------------------------------------------------------------

float AiServerShipObjectInterface::getMaximumYaw() const
{
	float const maximumYaw = ServerShipObjectInterface::getMaximumYaw();

	return maximumYaw;
}

// ----------------------------------------------------------------------

float AiServerShipObjectInterface::getYawAcceleration() const
{
	float const yawAcceleration = ServerShipObjectInterface::getYawAcceleration();

	return yawAcceleration;
}

// ----------------------------------------------------------------------

float AiServerShipObjectInterface::getMaximumPitch() const
{
	float const maximumPitch = ServerShipObjectInterface::getMaximumPitch();

	return maximumPitch;
}

// ----------------------------------------------------------------------

float AiServerShipObjectInterface::getPitchAcceleration() const
{
	float const pitchAcceleration = ServerShipObjectInterface::getPitchAcceleration();

	return pitchAcceleration;
}

// ----------------------------------------------------------------------

float AiServerShipObjectInterface::getMaximumRoll() const
{
	float const maximumRoll = ServerShipObjectInterface::getMaximumRoll();

	return maximumRoll;
}

// ----------------------------------------------------------------------

float AiServerShipObjectInterface::getRollAcceleration() const
{
	float const rollAcceleration = ServerShipObjectInterface::getRollAcceleration();

	return rollAcceleration;
}

// ----------------------------------------------------------------------

void AiServerShipObjectInterface::setFastAxisEnabled(bool const enabled)
{
	s_fastAxis = enabled;
}

// ----------------------------------------------------------------------

bool AiServerShipObjectInterface::isFastAxisEnabled()
{
	return s_fastAxis;
}

// ======================================================================
