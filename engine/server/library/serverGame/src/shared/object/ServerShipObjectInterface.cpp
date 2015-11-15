// ======================================================================
//
// ServerShipObjectInterface.cpp
// asommers
//
// copyright 2004, sony online entertainment
//
// ======================================================================

#include "serverGame/FirstServerGame.h"
#include "serverGame/ServerShipObjectInterface.h"

#include "serverGame/ShipObject.h"

// ======================================================================

ServerShipObjectInterface::ServerShipObjectInterface(ShipObject const * const shipObject) :
	ShipObjectInterface(),
	m_shipObject(shipObject)
{
}

// ----------------------------------------------------------------------

ServerShipObjectInterface::~ServerShipObjectInterface()
{
}

// ----------------------------------------------------------------------

float ServerShipObjectInterface::getMaximumSpeed() const
{
	return m_shipObject->getShipActualSpeedMaximum();
}

// ----------------------------------------------------------------------

float ServerShipObjectInterface::getSpeedAcceleration() const
{
	return m_shipObject->getShipActualAccelerationRate();
}

// ----------------------------------------------------------------------

float ServerShipObjectInterface::getSpeedDeceleration() const
{
	return m_shipObject->getShipActualDecelerationRate();
}

// ----------------------------------------------------------------------

float ServerShipObjectInterface::getSlideDampener() const
{
	return m_shipObject->getSlideDampener();
}

// ----------------------------------------------------------------------

float ServerShipObjectInterface::getMaximumYaw() const
{
	return m_shipObject->getShipActualYawRateMaximum();
}

// ----------------------------------------------------------------------

float ServerShipObjectInterface::getYawAcceleration() const
{
	return m_shipObject->getShipActualYawAccelerationRate();
}

// ----------------------------------------------------------------------

float ServerShipObjectInterface::getMaximumPitch() const
{
	return m_shipObject->getShipActualPitchRateMaximum();
}

// ----------------------------------------------------------------------

float ServerShipObjectInterface::getPitchAcceleration() const
{
	return m_shipObject->getShipActualPitchAccelerationRate();
}

// ----------------------------------------------------------------------

float ServerShipObjectInterface::getMaximumRoll() const
{
	return m_shipObject->getShipActualRollRateMaximum();
}

// ----------------------------------------------------------------------

float ServerShipObjectInterface::getRollAcceleration() const
{
	return m_shipObject->getShipActualRollAccelerationRate();
}

// ======================================================================
