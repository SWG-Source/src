// ======================================================================
//
// MiscPack.cpp
// copyright (c) 2001 Sony Online Entertainment
//
// Defines various packing and unpacking functions needed for SwgDatabaseServer
//
// ======================================================================

#include "SwgDatabaseServer/FirstSwgDatabaseServer.h"

#include "serverUtility/MissionLocation.h"
#include "sharedDatabaseInterface/Bindable.h"
#include "sharedDatabaseInterface/BindableNetworkId.h"
#include "sharedDatabaseInterface/BufferString.h"

// ======================================================================

void MissionLocation::unpackFromDatabase(const DB::BindableDouble &x,const DB::BindableDouble &y, const DB::BindableDouble &z, const DB::BindableString<500> &planet, const DB::BindableString<500> &region_name, const DB::BindableNetworkId &cellId)
{
	x.getValue(coordinates.x);
	y.getValue(coordinates.y);
	z.getValue(coordinates.z);
	planet.getValue(planetName);
	region_name.getValue(regionName);
	cellId.getValue(cell);
}

// ----------------------------------------------------------------------

void MissionLocation::packToDatabase(DB::BindableDouble &x,DB::BindableDouble &y, DB::BindableDouble &z, DB::BindableString<500> &planet, DB::BindableString<500> &region_name, DB::BindableNetworkId &cellId) const
{
	x.setValue(coordinates.x);
	y.setValue(coordinates.y);
	z.setValue(coordinates.z);
	planet.setValue(planetName);
	region_name.setValue(regionName);
	cellId.setValue(cell);
}

// ----------------------------------------------------------------------

void StringId::unpackFromDatabase(const DB::BufferString &table, const DB::BufferString &text)
{
	table.getValue(m_table);
	text.getValue(m_text);	
}

// ----------------------------------------------------------------------

void StringId::packToDatabase(DB::BufferString &table, DB::BufferString &text) const
{
	table.setValue(m_table);
	text.setValue(m_text);
}

// ======================================================================
