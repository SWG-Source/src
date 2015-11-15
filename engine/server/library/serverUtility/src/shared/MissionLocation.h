// ======================================================================
//
// MissionLocation.h
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_MissionLocation_H
#define INCLUDED_MissionLocation_H

// ======================================================================

#include "Archive/AutoByteStream.h"
#include "sharedFoundation/NetworkId.h"
#include "sharedMath/Vector.h"
#include "StringId.h"

// ======================================================================

namespace DB
{
	class BindableDouble;
	template <int S>
	class BindableString;
}

// ======================================================================

//Todo:  Generalize this into an all-purpose Location class
class MissionLocation
{
  public:
	Vector           coordinates;
	std::string      planetName;
	Unicode::String  regionName;
	NetworkId        cell;

  public:
	bool operator== (const MissionLocation &rhs) const;
	bool operator!= (const MissionLocation &rhs) const;

  public:
	//@todo: (un)pack cell
	void unpackFromDatabase(const DB::BindableDouble &x,const DB::BindableDouble &y, const DB::BindableDouble &z, const DB::BindableString<500> &planet, const DB::BindableString<500> &region_name, const DB::BindableNetworkId &cell);
	void packToDatabase(DB::BindableDouble &x,DB::BindableDouble &y, DB::BindableDouble &z, DB::BindableString<500> &planet, DB::BindableString<500> &region_name, DB::BindableNetworkId &cell) const;
};

// ======================================================================

inline bool MissionLocation::operator== (const MissionLocation &rhs) const
{
	return ((coordinates == rhs.coordinates) && (planetName == rhs.planetName) && (regionName == rhs.regionName) && (cell == rhs.cell));
}

// ----------------------------------------------------------------------

inline bool MissionLocation::operator!= (const MissionLocation &rhs) const
{
	return !(MissionLocation::operator==(rhs));
}

// ======================================================================

namespace Archive
{
	void get(ReadIterator & source, MissionLocation & target);
	void put(ByteStream & target, const MissionLocation & source);
}

// ======================================================================

#endif
