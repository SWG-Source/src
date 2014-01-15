//======================================================================
//
// ShipInternalDamageOverTime.h
// copyright (c) 2004 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_ShipInternalDamageOverTime_H
#define INCLUDED_ShipInternalDamageOverTime_H

#include "sharedObject/CachedNetworkId.h"

class ShipObject;

//======================================================================

class ShipInternalDamageOverTime
{
public:
	
	class ExpiredRemoveFunctor
	{
	public:
		bool operator() (ShipInternalDamageOverTime const & rhs)const;
	};

	ShipInternalDamageOverTime(ShipObject const & ship, int chassisSlot, float damageRate, float damageThreshold);

	float getDamageRate() const;
	float getDamageThreshold() const;
	int getChassisSlot() const;
	bool isExpired() const;

	void setDamageRate(float damageRate);
	void setDamageThreshold(float damageThreshold);

	bool applyDamage(float elapsedTime, float & damageApplied) const;
	bool checkValidity() const;
	bool checkValidity(float & hpCurrent, float & hpMaximum) const;
	void setExpired(bool expired) const;

	ShipObject * const getShipObject() const;
	bool operator<(ShipInternalDamageOverTime const & rhs) const;

private:

	CachedNetworkId m_shipId;
	int m_chassisSlot;
	float m_damageRate;
	float m_damageThreshold;

	mutable bool m_expired;
};

//----------------------------------------------------------------------

inline float ShipInternalDamageOverTime::getDamageRate() const
{
	return m_damageRate;
}

//----------------------------------------------------------------------

inline float ShipInternalDamageOverTime::getDamageThreshold() const
{
	return m_damageThreshold;
}

//----------------------------------------------------------------------

inline int ShipInternalDamageOverTime::getChassisSlot() const
{
	return m_chassisSlot;
}

//----------------------------------------------------------------------

inline bool ShipInternalDamageOverTime::isExpired() const
{
	return m_expired;
}

//----------------------------------------------------------------------

inline bool ShipInternalDamageOverTime::operator<(ShipInternalDamageOverTime const & rhs) const
{
	if (m_shipId < rhs.m_shipId)
		return true;

	if (m_shipId == rhs.m_shipId && m_chassisSlot < rhs.m_chassisSlot)
		return true;

	return false;
}

//----------------------------------------------------------------------

inline bool ShipInternalDamageOverTime::ExpiredRemoveFunctor::operator() (ShipInternalDamageOverTime const & rhs)const
{
	return rhs.isExpired();
}

//----------------------------------------------------------------------

inline bool ShipInternalDamageOverTime::checkValidity() const
{
	float hpCur = 0.0f;
	float hpMax = 0.0f;
	return checkValidity(hpCur, hpMax);
}

//======================================================================

#endif
