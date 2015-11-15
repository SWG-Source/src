// ======================================================================
//
// TravelPoint.cpp
// asommers
//
// copyright 2002, sony online entertainment
//
// ======================================================================

#include "sharedGame/FirstSharedGame.h"
#include "sharedGame/TravelPoint.h"

#include "Archive/Archive.h"
#include "Archive/ByteStream.h"
#include "sharedFoundation/NetworkIdArchive.h"
#include "sharedMathArchive/VectorArchive.h"

// ======================================================================
// PUBLIC TravelPoint
// ======================================================================

TravelPoint::TravelPoint () :
	m_name (),
	m_gcwContestedRegion (),
	m_position_w (),
	m_cost (0),
	m_interplanetary (false),
	m_type (TPT_Unknown)
{
}

// ----------------------------------------------------------------------

TravelPoint::TravelPoint (const std::string& name, const std::string& gcwContestedRegion, const Vector& position_w, const int cost, const bool interplanetary, const TravelPointType type) :
	m_name (name),
	m_gcwContestedRegion (gcwContestedRegion),
	m_position_w (position_w),
	m_cost (cost),
	m_interplanetary (interplanetary),
	m_type (type)
{
}

// ----------------------------------------------------------------------

TravelPoint::TravelPoint (const TravelPoint& rhs) :
	m_name (rhs.m_name),
	m_gcwContestedRegion (rhs.m_gcwContestedRegion),
	m_position_w (rhs.m_position_w),
	m_cost (rhs.m_cost),
	m_interplanetary (rhs.m_interplanetary),
	m_type (rhs.m_type)
{
}

// ----------------------------------------------------------------------

TravelPoint& TravelPoint::operator= (const TravelPoint& rhs)
{
	if (this != &rhs)
	{
		m_name = rhs.m_name;
		m_gcwContestedRegion = rhs.m_gcwContestedRegion;
		m_position_w = rhs.m_position_w;
		m_cost = rhs.m_cost;
		m_interplanetary = rhs.m_interplanetary;
		m_type = rhs.m_type;
	}

	return *this;
}

// ----------------------------------------------------------------------

TravelPoint::~TravelPoint ()
{
}

// ----------------------------------------------------------------------

bool TravelPoint::operator== (const TravelPoint& rhs) const
{
	return 
		m_name == rhs.m_name && 
		m_gcwContestedRegion == rhs.m_gcwContestedRegion &&
		m_position_w == rhs.m_position_w &&
		m_cost == rhs.m_cost &&
		m_interplanetary == rhs.m_interplanetary &&
		m_type == rhs.m_type;
}

// ======================================================================

namespace Archive
{
	void get(ReadIterator& source, TravelPoint& target)
	{
		Archive::get (source, target.m_name);
		Archive::get (source, target.m_gcwContestedRegion);
		Archive::get (source, target.m_position_w);
		Archive::get (source, target.m_cost);
		Archive::get (source, target.m_interplanetary);

		uint32 type = 0;
		Archive::get (source, type);
		target.m_type = (static_cast<TravelPoint::TravelPointType>(type));
	}

	void put(ByteStream& target, const TravelPoint& source)
	{
		Archive::put (target, source.m_name);
		Archive::put (target, source.m_gcwContestedRegion);
		Archive::put (target, source.m_position_w);
		Archive::put (target, source.m_cost);
		Archive::put (target, source.m_interplanetary);
		Archive::put (target, static_cast<uint32>(source.m_type));
	}
}

// ======================================================================
