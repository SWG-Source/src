// ======================================================================
//
// CityInfo.cpp
//
// Copyright 2002 Sony Online Entertainment
//
// ======================================================================

#include "serverGame/FirstServerGame.h"
#include "serverGame/CityInfo.h"

// ======================================================================

bool CityInfo::operator== (const CityInfo& rhs) const
{
	if (this == &rhs)
		return true;

	return ((m_cityName == rhs.m_cityName) &&
		(m_cityHallId == rhs.m_cityHallId) &&
		(m_planet == rhs.m_planet) &&
		(m_x == rhs.m_x) &&
		(m_z == rhs.m_z) &&
		(m_radius == rhs.m_radius) &&
		(m_faction == rhs.m_faction) &&
		(m_gcwDefenderRegion == rhs.m_gcwDefenderRegion) &&
		(m_timeJoinedGcwDefenderRegion == rhs.m_timeJoinedGcwDefenderRegion) &&
		(m_creationTime == rhs.m_creationTime) &&
		(m_leaderId == rhs.m_leaderId) &&
		(m_citizenCount == rhs.m_citizenCount) &&
		(m_structureCount == rhs.m_structureCount) &&
		(m_incomeTax == rhs.m_incomeTax) &&
		(m_propertyTax == rhs.m_propertyTax) &&
		(m_salesTax == rhs.m_salesTax) &&
		(m_travelLoc == rhs.m_travelLoc) &&
		(m_travelCost == rhs.m_travelCost) &&
		(m_travelInterplanetary == rhs.m_travelInterplanetary) &&
		(m_cloneLoc == rhs.m_cloneLoc) &&
		(m_cloneRespawn == rhs.m_cloneRespawn) &&
		(m_cloneRespawnCell == rhs.m_cloneRespawnCell) &&
		(m_cloneId == rhs.m_cloneId));
}

// ----------------------------------------------------------------------

bool CityInfo::operator!= (const CityInfo& rhs) const
{
	return !operator==(rhs);
}

// ======================================================================
