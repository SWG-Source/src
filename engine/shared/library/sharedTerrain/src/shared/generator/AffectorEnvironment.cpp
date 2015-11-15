//===================================================================
//
// AffectorEnvironment.cpp
// asommers 9-30-2000
//
// copyright 2000, verant interactive
//
//===================================================================

#include "sharedTerrain/FirstSharedTerrain.h"
#include "sharedTerrain/AffectorEnvironment.h"

#include "sharedTerrain/Affector.h"
#include "sharedFile/Iff.h"

//-------------------------------------------------------------------
//
// AffectorEnvironment
//
AffectorEnvironment::AffectorEnvironment () :
	TerrainGenerator::Affector (TAG_AENV, TGAT_environment),
	m_cachedFamilyId (-1),
	m_cachedEgi (),
	m_cachedFeatherClamp (0.f),
	m_familyId (0),
	m_useFeatherClampOverride (false),
	m_featherClampOverride (1.f)
{
}

//-------------------------------------------------------------------

AffectorEnvironment::~AffectorEnvironment ()
{
}

//-------------------------------------------------------------------

void AffectorEnvironment::setFamilyId (const int familyId)
{
	m_familyId = familyId;
}

//-------------------------------------------------------------------

void AffectorEnvironment::setUseFeatherClampOverride (const bool useFeatherClampOverride)
{
	m_useFeatherClampOverride = useFeatherClampOverride;
}

//-------------------------------------------------------------------

void AffectorEnvironment::setFeatherClampOverride (const float featherClampOverride)
{
	m_featherClampOverride = featherClampOverride;
}

//-------------------------------------------------------------------

void AffectorEnvironment::prepare ()
{
	m_cachedFamilyId = -1;
}

//-------------------------------------------------------------------

unsigned AffectorEnvironment::getAffectedMaps() const
{
	return TGM_environment;
}

//-------------------------------------------------------------------

void AffectorEnvironment::affect (const float /*worldX*/, const float /*worldZ*/, const int x, const int z, const float amount, const TerrainGenerator::GeneratorChunkData& generatorChunkData) const
{
	if (amount > 0.f)
	{
		if (m_cachedFamilyId != m_familyId)
		{
			m_cachedFamilyId = m_familyId;
			m_cachedEgi = generatorChunkData.environmentGroup->chooseEnvironment (m_familyId);
			m_cachedFeatherClamp = generatorChunkData.environmentGroup->getFamilyFeatherClamp (m_familyId);
		}

		const float featherClamp = m_useFeatherClampOverride ? m_featherClampOverride : m_cachedFeatherClamp;

		if (amount >= featherClamp)
		{
			EnvironmentGroup::Info egi = m_cachedEgi;
			generatorChunkData.environmentMap->setData (x, z, egi);
		}
	}
}

//-------------------------------------------------------------------

void AffectorEnvironment::load (Iff& iff)
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
			DEBUG_FATAL (true, ("invalid AffectorEnvironment version %s/%s", buffer, tagBuffer));
		}
		break;
	}
}

//-------------------------------------------------------------------

void AffectorEnvironment::load_0000 (Iff& iff)
{
	iff.enterForm (TAG_0000);

		//-- load the base data
		LayerItem::load (iff);

		//-- load specific data
		iff.enterChunk (TAG_DATA);

			setFamilyId (iff.read_int32 ());
			setUseFeatherClampOverride (iff.read_int32 () != 0);
			setFeatherClampOverride (iff.read_float ());

		iff.exitChunk (TAG_DATA);

	iff.exitForm (TAG_0000);
}

//-------------------------------------------------------------------

void AffectorEnvironment::save (Iff& iff) const
{
	iff.insertForm (TAG_0000);

		//-- save the base
		LayerItem::save (iff);

		//-- save specific data
		iff.insertChunk (TAG_DATA);

			iff.insertChunkData (getFamilyId ());
			iff.insertChunkData (getUseFeatherClampOverride () ? static_cast<int32> (1) : static_cast<int32> (0));
			iff.insertChunkData (getFeatherClampOverride ());

		iff.exitChunk (TAG_DATA);

	iff.exitForm (TAG_0000);
}

//===================================================================

