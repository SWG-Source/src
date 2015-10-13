// ======================================================================
//
// TravelPoint.h
// asommers
//
// copyright 2002, sony online entertainment
//
// ======================================================================

#ifndef INCLUDED_TravelPoint_H
#define INCLUDED_TravelPoint_H

// ======================================================================

#include "sharedMath/Vector.h"

class TravelPoint;

// ======================================================================

namespace Archive
{
	class ReadIterator;
	void get(ReadIterator& source, TravelPoint& target);

	class ByteStream;
	void put(ByteStream& target, const TravelPoint& source);
}

// ======================================================================

class TravelPoint
{
	friend void Archive::get(Archive::ReadIterator& source, TravelPoint& target);
	friend void Archive::put(Archive::ByteStream& target, const TravelPoint& source);

public:

	// *****WARNING*****
	// this must be kept in sync with the corresponding definitions in base_class.java
	// *****WARNING*****
	enum TravelPointType
	{
		TPT_Unknown              = 0x00000000,
		TPT_NPC_Starport         = 0x00000001,
		TPT_NPC_Shuttleport      = 0x00000002,
		TPT_NPC_StaticBaseBeacon = 0x00000004,
		TPT_PC_Shuttleport       = 0x00000008,
		TPT_PC_CampShuttleBeacon = 0x00000010,
	};

	TravelPoint ();
	TravelPoint (const std::string& name, const std::string& gcwContestedRegion, const Vector& position_w, int cost, bool interplanetary, TravelPointType type);
	TravelPoint (const TravelPoint& rhs);
	TravelPoint& operator= (const TravelPoint& rhs);
	~TravelPoint ();

	bool operator== (const TravelPoint& rhs) const;

	const std::string& getName () const;
	const std::string& getGcwContestedRegion () const;
	const Vector&      getPosition_w () const;
	int                getCost () const;
	bool               getInterplanetary () const;
	TravelPointType    getType () const;

private:

	std::string m_name;
	std::string m_gcwContestedRegion;
	Vector      m_position_w;
	int         m_cost;
	bool        m_interplanetary;
	TravelPointType m_type;
};

// ----------------------------------------------------------------------

inline const std::string& TravelPoint::getName () const
{
	return m_name;
}

// ----------------------------------------------------------------------

inline const std::string& TravelPoint::getGcwContestedRegion () const
{
	return m_gcwContestedRegion;
}

// ----------------------------------------------------------------------

inline const Vector& TravelPoint::getPosition_w () const
{
	return m_position_w;
}

// ----------------------------------------------------------------------

inline int TravelPoint::getCost () const
{
	return m_cost;
}

// ----------------------------------------------------------------------

inline bool TravelPoint::getInterplanetary () const
{
	return m_interplanetary;
}

// ----------------------------------------------------------------------

inline TravelPoint::TravelPointType TravelPoint::getType () const
{
	return m_type;
}

// ======================================================================

#endif
