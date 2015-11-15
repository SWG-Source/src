// ======================================================================
//
// AffectorPassable.cpp
// Copyright 2004 Sony Online Entertainment
//
// ======================================================================

#include "sharedTerrain/FirstSharedTerrain.h"
#include "sharedTerrain/AffectorPassable.h"

#include "sharedFile/Iff.h"

//-------------------------------------------------------------------
//
// AffectorExclude
//
AffectorPassable::AffectorPassable () :
TerrainGenerator::Affector (TAG_APAS, TGAT_passable),
m_passable(true),
m_featherThreshold(0.0f)
{
}

//-------------------------------------------------------------------

AffectorPassable::~AffectorPassable ()
{
}

//-------------------------------------------------------------------

unsigned AffectorPassable::getAffectedMaps() const
{
	return TGM_passable;
}

//-------------------------------------------------------------------

void AffectorPassable::affect (const float /*worldX*/, const float /*worldZ*/, const int x, const int z, const float amount, const TerrainGenerator::GeneratorChunkData& generatorChunkData) const
{
	if (amount >= m_featherThreshold)
		generatorChunkData.passableMap->setData (x, z, m_passable);
}

//-------------------------------------------------------------------

void AffectorPassable::load (Iff& iff)
{
	switch (iff.getCurrentName ())
	{
	case TAG_0000:
		load_0000 (iff);
		break;

	default:
		{
			char tagBuffer [5];
			ConvertTagToString (iff.getCurrentName (), tagBuffer);

			char buffer [128];
			iff.formatLocation (buffer, sizeof (buffer));
			DEBUG_FATAL (true, ("invalid AffectorPassable version %s/%s", buffer, tagBuffer));
		}
		break;
	}
}

//-------------------------------------------------------------------

void AffectorPassable::load_0000 (Iff& iff)
{
	iff.enterForm (TAG_0000);
	{
		//-- load the base data
		LayerItem::load (iff);

		//-- load specific data
		iff.enterChunk (TAG_DATA);
		{
			m_passable = iff.read_bool8();
			m_featherThreshold = iff.read_float();
		}
		iff.exitChunk (TAG_DATA);
	}
	iff.exitForm (TAG_0000);
}

//-------------------------------------------------------------------

void AffectorPassable::save (Iff& iff) const
{
	iff.insertForm (TAG_0000);
	{
		//-- save the base
		LayerItem::save (iff);

		//-- save specific data
		iff.insertChunk (TAG_DATA);
		{
			iff.insertChunkData(m_passable);
			iff.insertChunkData(m_featherThreshold);
		}
		iff.exitChunk (TAG_DATA);
	}
	iff.exitForm (TAG_0000);
}

//----------------------------------------------------------------------

bool AffectorPassable::isPassable() const
{
	return m_passable;
}

//----------------------------------------------------------------------

float AffectorPassable::getFeatherThreshold() const
{
	return m_featherThreshold;
}

//----------------------------------------------------------------------

void AffectorPassable::setPassable(bool const passable)
{
	m_passable = passable;
}

//----------------------------------------------------------------------

void AffectorPassable::setFeatherThreshold(float const featherThreshold)
{
	m_featherThreshold = featherThreshold;
}

//-------------------------------------------------------------------

