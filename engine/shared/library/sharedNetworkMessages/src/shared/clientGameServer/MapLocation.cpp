// MapLocation.cpp
// Copyright 2000-02, Sony Online Entertainment Inc., all rights reserved

#include "sharedNetworkMessages/FirstSharedNetworkMessages.h"
#include "sharedNetworkMessages/MapLocation.h"

//-----------------------------------------------------------------------

MapLocation::MapLocation() :
m_locationId   (),
m_locationName (),
m_location     (),
m_category     (0),
m_subCategory  (0),
m_flags        (0),
m_size		   (0.0f)
{
}

//----------------------------------------------------------------------

MapLocation::MapLocation(const MapLocation &rhs) :
m_locationId   (rhs.m_locationId),
m_locationName (rhs.m_locationName),
m_location     (rhs.m_location),
m_category     (rhs.m_category),
m_subCategory  (rhs.m_subCategory),
m_flags        (rhs.m_flags),
m_size		   (rhs.m_size)
{
}

//----------------------------------------------------------------------

MapLocation::MapLocation(const NetworkId &locationId, const Unicode::String &locationName, const Vector2d &location, uint8 category, uint8 subCategory, uint8 flags, float size) :
m_locationId   (locationId),
m_locationName (locationName),
m_location     (location),
m_category     (category),
m_subCategory  (subCategory),
m_flags        (flags),
m_size		   (size)
{

}

//----------------------------------------------------------------------

MapLocation & MapLocation::operator= (const MapLocation &rhs)
{
	if (this != &rhs)
	{
		m_locationId   = rhs.m_locationId;
		m_locationName = rhs.m_locationName;
		m_location     = rhs.m_location;
		m_category     = rhs.m_category;
		m_subCategory  = rhs.m_subCategory;
		m_flags        = rhs.m_flags;
		m_size		   = rhs.m_size;
	}
	return *this;
}


//----------------------------------------------------------------------
