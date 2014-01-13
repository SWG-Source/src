// MapLocation.h
// Copyright 2000-02, Sony Online Entertainment Inc., all rights reserved. 

#ifndef	_INCLUDED_MapLocation_H
#define	_INCLUDED_MapLocation_H

//-----------------------------------------------------------------------

#include "sharedFoundation/NetworkId.h"
#include "sharedMath/Vector2d.h"
#include "Unicode.h"

//-----------------------------------------------------------------------

class MapLocation
{
public:

	enum Flags
	{
		F_inactive = 0x0001,
		F_active   = 0x0002,
	};

	MapLocation & operator = (const MapLocation & rhs);
	bool operator== (const MapLocation & rhs) const;
	bool operator!= (const MapLocation & rhs) const;
//	bool operator<  (const MapLocation & rhs) const;
	MapLocation();
	MapLocation(const MapLocation & source);
	MapLocation(const NetworkId &locationId, const Unicode::String &locationName, const Vector2d &location, uint8 category, uint8 subCategory, uint8 flags, float size = 0.0f);

	const NetworkId &          getLocationId        () const;
	const Unicode::String &    getLocationName      () const;
	const Vector2d &           getLocation          () const;
	const uint8                getCategory          () const;
	const uint8                getSubCategory       () const;
	const uint8                getFlags             () const;
	const bool                 isActive             () const;
	const bool                 isInactive           () const;
	const float				   getSize              () const;

	NetworkId             m_locationId;
	Unicode::String       m_locationName;
	Vector2d              m_location;
	uint8                 m_category;
	uint8                 m_subCategory;
	uint8                 m_flags;
	float				  m_size;
};

//----------------------------------------------------------------------

inline const NetworkId          & MapLocation::getLocationId        () const {return m_locationId;}
inline const Unicode::String    & MapLocation::getLocationName      () const {return m_locationName;}
inline const Vector2d           & MapLocation::getLocation          () const {return m_location;}
inline const uint8                MapLocation::getCategory          () const {return m_category;}
inline const uint8                MapLocation::getSubCategory       () const {return m_subCategory;}
inline const uint8                MapLocation::getFlags             () const {return m_flags;}
inline const float				  MapLocation::getSize              () const {return m_size;}

//----------------------------------------------------------------------


inline bool MapLocation::operator== (const MapLocation & rhs) const
{
	return 
		m_locationId   == rhs.m_locationId &&
		m_locationName == rhs.m_locationName &&
		m_location     == rhs.m_location &&
		m_category     == rhs.m_category &&
		m_subCategory  == rhs.m_subCategory &&
		m_flags        == rhs.m_flags;
}

//----------------------------------------------------------------------

inline bool MapLocation::operator!= (const MapLocation & rhs) const
{
	return !(*this == rhs);
}

//----------------------------------------------------------------------

inline const bool MapLocation::isInactive           () const
{
	return (m_flags & F_inactive) != 0;
}

//----------------------------------------------------------------------

inline const bool MapLocation::isActive           () const
{
	return (m_flags & F_active) != 0;
}

//-----------------------------------------------------------------------

#endif	// _INCLUDED_MapLocation_H
