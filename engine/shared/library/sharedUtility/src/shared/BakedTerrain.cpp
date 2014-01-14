//===================================================================
//
// BakedTerrain.cpp
// asommers
//
// copyright 2002, sony online entertainment
//
//===================================================================

#include "sharedUtility/FirstSharedUtility.h"
#include "sharedUtility/BakedTerrain.h"

#include "sharedFile/Iff.h"
#include "sharedMath/Rectangle2d.h"
#include "sharedMath/Vector2d.h"

#include <algorithm>

//===================================================================

namespace
{
	const Tag TAG_WMAP = TAG (W,M,A,P);
	const Tag TAG_SMAP = TAG (S,M,A,P);
}

//===================================================================
// PUBLIC BakedTerrain
//===================================================================

BakedTerrain::BakedTerrain () :
	m_mapWidthInMeters (0.f),
	m_chunkWidthInMeters (0.f),
	m_width (0),
	m_height (0),
	m_waterMap (0),
	m_slopeMap (0)
{
}

//-------------------------------------------------------------------

BakedTerrain::~BakedTerrain ()
{
	destroy ();

	m_waterMap = 0;
	m_slopeMap = 0;
}

//-------------------------------------------------------------------

void BakedTerrain::load (Iff& iff)
{
	destroy ();

	switch (iff.getCurrentName ())
	{
	case TAG_0000:
		load_0000 (iff);
		break;

	case TAG_0001:
		load_0001 (iff);
		break;

	default:
		{
			char tagBuffer [5];
			ConvertTagToString (iff.getCurrentName (), tagBuffer);

			char buffer [128];
			iff.formatLocation (buffer, sizeof (buffer));
			DEBUG_FATAL (true, ("invalid BakedTerrain version %s/%s", buffer, tagBuffer));
		}
		break;
	}
}

//-------------------------------------------------------------------

void BakedTerrain::save (Iff& iff) const
{
	iff.insertForm (TAG_0001);

		iff.insertChunk (TAG_DATA);
			iff.insertChunkData (m_mapWidthInMeters);
			iff.insertChunkData (m_chunkWidthInMeters);
			iff.insertChunkData (m_width);
			iff.insertChunkData (m_height);
		iff.exitChunk (TAG_DATA);
		
		iff.insertChunk (TAG_WMAP);
			iff.insertChunkData (m_waterMap, m_width * m_height);
		iff.exitChunk (TAG_WMAP);

		iff.insertChunk (TAG_SMAP);
			iff.insertChunkData (m_slopeMap, m_width * m_height);
		iff.exitChunk (TAG_SMAP);

	iff.exitForm (TAG_0001);
}

//-------------------------------------------------------------------

float BakedTerrain::getMapWidthInMeters () const
{
	return m_mapWidthInMeters;
}

//-------------------------------------------------------------------

float BakedTerrain::getChunkWidthInMeters () const
{
	return m_chunkWidthInMeters;
}

//-------------------------------------------------------------------

bool BakedTerrain::getWater (const int chunkX, const int chunkY) const
{
	if (!m_waterMap)
		return false;

	const int x = chunkX + static_cast<int> ((m_mapWidthInMeters / m_chunkWidthInMeters) * 0.5f);
	const int y = chunkY + static_cast<int> ((m_mapWidthInMeters / m_chunkWidthInMeters) * 0.5f);

	return getBool (m_waterMap, x, y);
}

//-------------------------------------------------------------------

bool BakedTerrain::getSlope (const int chunkX, const int chunkY) const
{
	if (!m_slopeMap)
		return false;

	const int x = chunkX + static_cast<int> ((m_mapWidthInMeters / m_chunkWidthInMeters) * 0.5f);
	const int y = chunkY + static_cast<int> ((m_mapWidthInMeters / m_chunkWidthInMeters) * 0.5f);

	return getBool (m_slopeMap, x, y);
}

//-------------------------------------------------------------------

bool BakedTerrain::getWater (const Vector2d& position) const
{
	if (!m_waterMap)
		return false;

	int x;
	int y;
	calculateMapXy (position, x, y);
	
	return getBool (m_waterMap, x, y);
}

//-------------------------------------------------------------------

bool BakedTerrain::getSlope (const Vector2d& position) const
{
	if (!m_slopeMap)
		return false;

	int x;
	int y;
	calculateMapXy (position, x, y);
	
	return getBool (m_slopeMap, x, y);
}

//-------------------------------------------------------------------

bool BakedTerrain::getWater (const Rectangle2d& rectangle) const
{
	return getMap (m_waterMap, rectangle);
}

//-------------------------------------------------------------------

bool BakedTerrain::getSlope (const Rectangle2d& rectangle) const
{
	return getMap (m_slopeMap, rectangle);
}

//-------------------------------------------------------------------

void BakedTerrain::setMap (const float mapWidthInMeters, const float chunkWidthInMeters)
{
	destroy ();

	m_mapWidthInMeters = mapWidthInMeters;
	m_chunkWidthInMeters = chunkWidthInMeters;
	m_width = static_cast<int> (m_mapWidthInMeters / m_chunkWidthInMeters) / 8;
	m_height = static_cast<int> (m_mapWidthInMeters / m_chunkWidthInMeters);
	const int size = m_width * m_height;
	m_waterMap = new uint8 [size];
	memset (m_waterMap, 0, size);
	m_slopeMap = new uint8 [size];
	memset (m_slopeMap, 0, size);
}

//-------------------------------------------------------------------

void BakedTerrain::setWater (int chunkX, int chunkY, bool water)
{
	if (!m_waterMap)
		return;

	const int x = chunkX + static_cast<int> ((m_mapWidthInMeters / m_chunkWidthInMeters) * 0.5f);
	const int y = chunkY + static_cast<int> ((m_mapWidthInMeters / m_chunkWidthInMeters) * 0.5f);

	setBool (m_waterMap, x, y, water);
}

//-------------------------------------------------------------------

void BakedTerrain::setSlope (int chunkX, int chunkY, bool slope)
{
	if (!m_slopeMap)
		return;

	const int x = chunkX + static_cast<int> ((m_mapWidthInMeters / m_chunkWidthInMeters) * 0.5f);
	const int y = chunkY + static_cast<int> ((m_mapWidthInMeters / m_chunkWidthInMeters) * 0.5f);

	setBool (m_slopeMap, x, y, slope);
}

//===================================================================
// PRIVATE BakedTerrain
//===================================================================

void BakedTerrain::destroy ()
{
	delete [] m_waterMap;
	m_waterMap = 0;

	delete [] m_slopeMap;
	m_slopeMap = 0;
}

//-------------------------------------------------------------------

int BakedTerrain::calculateChunkX (const Vector2d& position) const
{
	const int chunkX = static_cast<int> ((position.x >= 0.f) ? floorf (position.x / m_chunkWidthInMeters) : ceilf (position.x / m_chunkWidthInMeters));

	return (position.x < 0.f) ? chunkX - 1 : chunkX;
}

//-------------------------------------------------------------------

int BakedTerrain::calculateChunkY (const Vector2d& position) const
{
	const int chunkY = static_cast<int> ((position.y >= 0.f) ? floorf (position.y / m_chunkWidthInMeters) : ceilf (position.y / m_chunkWidthInMeters));
   
   	return (position.y < 0.f) ? chunkY - 1 : chunkY;
}

//-------------------------------------------------------------------

void BakedTerrain::calculateMapXy (const Vector2d& position, int& x, int& y) const
{
	x = calculateChunkX (position) + static_cast<int> ((m_mapWidthInMeters / m_chunkWidthInMeters) * 0.5f);
	y = calculateChunkY (position) + static_cast<int> ((m_mapWidthInMeters / m_chunkWidthInMeters) * 0.5f);
}

//-------------------------------------------------------------------

bool BakedTerrain::getBool (const uint8* const map, const int mapX, const int mapY) const
{
	const int index  = mapX >> 3;
	const int offset = mapX % 8;

	if (index < 0 || index >= m_width || mapY < 0 || mapY >= m_height)
		return false;

	return (map [mapY * m_width + index] & (1 << offset)) != 0;
}

//-------------------------------------------------------------------

bool BakedTerrain::getMap (const uint8* const map, const Rectangle2d& rectangle) const
{
	if (!map)
		return false;

	int x0;
	int y0;
	calculateMapXy (Vector2d (rectangle.x0, rectangle.y0), x0, y0);

	int x1;
	int y1;
	calculateMapXy (Vector2d (rectangle.x1, rectangle.y1), x1, y1);

	if (x0 > x1)
		std::swap (x0, x1);

	if (y0 > y1)
		std::swap (y0, y1);

	int x;
	int y;
	for (y = y0; y <= y1; ++y)
		for (x = x0; x <= x1; ++x)
			if (getBool (map, x, y))
				return true;

	return false;
}

//-------------------------------------------------------------------

void BakedTerrain::setBool (uint8* const map, const int mapX, const int mapY, const bool value)
{
	const int index  = mapX >> 3;
	const int offset = mapX % 8;

	VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE (0, index, m_width);
	VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE (0, mapY, m_height);

	if (value)
		map [mapY * m_width + index] |= (1 << offset);
	else
		map [mapY * m_width + index] &= ~(1 << offset);
}

//-------------------------------------------------------------------

void BakedTerrain::load_0000 (Iff& iff)
{
	iff.enterForm (TAG_0000);
	iff.exitForm (TAG_0000, true);
}

//-------------------------------------------------------------------

void BakedTerrain::load_0001 (Iff& iff)
{
	iff.enterForm (TAG_0001);

		iff.enterChunk (TAG_DATA);
			m_mapWidthInMeters = iff.read_float ();
			m_chunkWidthInMeters = iff.read_float ();
			m_width  = iff.read_int32 ();
			m_height = iff.read_int32 ();
		iff.exitChunk (TAG_DATA);
		
		iff.enterChunk (TAG_WMAP);
			m_waterMap = iff.readRest_uint8 ();
		iff.exitChunk (TAG_WMAP);

		iff.enterChunk (TAG_SMAP);
			m_slopeMap = iff.readRest_uint8 ();
		iff.exitChunk (TAG_SMAP);

	iff.exitForm (TAG_0001);
}

//===================================================================

