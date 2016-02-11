//======================================================================
//
// ShipInternalDamageOverTime.cpp
// copyright (c) 2004 Sony Online Entertainment
//
//======================================================================

#include "serverGame/FirstServerGame.h"
#include "serverGame/ShipInternalDamageOverTime.h"

#include "serverGame/ShipObject.h"
#include "sharedGame/ShipChassisSlotType.h"

//======================================================================

ShipInternalDamageOverTime::ShipInternalDamageOverTime(ShipObject const & ship, int chassisSlot, float damageRate, float damageThreshold) :
m_shipId(const_cast<ShipObject &>(ship)),
m_chassisSlot(chassisSlot),
m_damageRate(damageRate),
m_damageThreshold(damageThreshold),
m_expired(false)
{
}

//----------------------------------------------------------------------

void ShipInternalDamageOverTime::setDamageRate(float damageRate)
{
	m_damageRate = damageRate;
}

//----------------------------------------------------------------------

void ShipInternalDamageOverTime::setDamageThreshold(float damageThreshold)
{
	m_damageThreshold = damageThreshold;
}

//----------------------------------------------------------------------

ShipObject * const ShipInternalDamageOverTime::getShipObject() const
{
	Object  * const object = m_shipId.getObject();
	if (object != nullptr && object->isAuthoritative())
	{
		ServerObject  * const serverObject = object->asServerObject();
		if (serverObject != nullptr)
			return serverObject->asShipObject();
	}
	
	return nullptr;
}

//----------------------------------------------------------------------

bool ShipInternalDamageOverTime::checkValidity(float & hpCurrent, float & hpMaximum) const
{
	ShipObject * const ship = getShipObject();
	if (ship == nullptr)
		return false;

	if (m_chassisSlot < 0 || m_chassisSlot > ShipChassisSlotType::SCST_num_types)
		return false;
		
	//- the ship is the damage target
	if (m_chassisSlot == ShipChassisSlotType::SCST_num_types)
	{
		hpCurrent = ship->getCurrentChassisHitPoints();
		hpMaximum = ship->getMaximumChassisHitPoints();
	}
	else
	{
		if (!ship->isSlotInstalled(m_chassisSlot))
			return false;
		
		hpCurrent = ship->getComponentHitpointsCurrent(m_chassisSlot);
		hpMaximum = ship->getComponentHitpointsMaximum(m_chassisSlot);
	}
	
	if (hpMaximum <= 0.0f)
		return false;

	return true;
}

//----------------------------------------------------------------------
/**
* @return false if the IDOT is below the damage threshold, or the ship/component no longer exists
*/

bool ShipInternalDamageOverTime::applyDamage(float elapsedTime, float & damageApplied) const
{
	ShipObject * const ship = getShipObject();
	if (ship == nullptr)
		return false;

	float hpCurrent = 0.0f;
	float hpMaximum = 0.0f;

	if (!checkValidity(hpCurrent, hpMaximum))
		return false;

	float const minHpCurrent = m_damageThreshold * hpMaximum;
	float const newHpCurrent = clamp(minHpCurrent, hpCurrent - (elapsedTime * m_damageRate), hpMaximum);

	if (m_chassisSlot == ShipChassisSlotType::SCST_num_types)
	{
		if (!ship->setCurrentChassisHitPoints(newHpCurrent))
			return false;
	}
	else
	{
		if (!ship->setComponentHitpointsCurrent(m_chassisSlot, newHpCurrent))
			return false;
	}
	
	damageApplied = hpCurrent - newHpCurrent;

	float const newHpPercent = newHpCurrent / hpMaximum;

	if (newHpPercent <= m_damageThreshold)
		return false;

	return true;
}

//----------------------------------------------------------------------

void ShipInternalDamageOverTime::setExpired(bool expired) const
{
	m_expired = expired;
}

//======================================================================
