//
// AffectorShader.cpp
// asommers 9-30-2000
//
// copyright 2000, verant interactive
//

//-------------------------------------------------------------------

#include "sharedTerrain/FirstSharedTerrain.h"
#include "sharedTerrain/AffectorShader.h"

#include "sharedTerrain/Affector.h"
#include "sharedTerrain/CoordinateHash.h"
#include "sharedRandom/FastRandomGenerator.h"
#include "sharedFile/Iff.h"

//-------------------------------------------------------------------
//
// AffectorShaderConstant
//
AffectorShaderConstant::AffectorShaderConstant () :
	TerrainGenerator::Affector (TAG_ASCN, TGAT_shaderConstant),
	m_cachedFamilyId (-1),
	m_cachedSgi (),
	m_cachedFeatherClamp (0.f),
	m_familyId (0),
	m_useFeatherClampOverride (false),
	m_featherClampOverride (1.f)
{
}

//-------------------------------------------------------------------

AffectorShaderConstant::~AffectorShaderConstant ()
{
}

//-------------------------------------------------------------------

void AffectorShaderConstant::setFamilyId (const int familyId)
{
	m_familyId = familyId;
}

//-------------------------------------------------------------------

void AffectorShaderConstant::setUseFeatherClampOverride (const bool useFeatherClampOverride)
{
	m_useFeatherClampOverride = useFeatherClampOverride;
}

//-------------------------------------------------------------------

void AffectorShaderConstant::setFeatherClampOverride (const float featherClampOverride)
{
	m_featherClampOverride = featherClampOverride;
}

//-------------------------------------------------------------------

bool AffectorShaderConstant::affectsShader () const
{
	return true;
}

//-------------------------------------------------------------------

void AffectorShaderConstant::prepare ()
{
	m_cachedFamilyId = -1;
}

//-------------------------------------------------------------------

unsigned AffectorShaderConstant::getAffectedMaps() const
{
	return TGM_shader;
}

//-------------------------------------------------------------------

void AffectorShaderConstant::affect (const float worldX, const float worldZ, const int x, const int z, const float amount, const TerrainGenerator::GeneratorChunkData& generatorChunkData) const
{
	if (generatorChunkData.m_legacyRandomGenerator)
	{
		_legacyAffect(worldX, worldZ, x, z, amount, generatorChunkData);
		return;
	}
	if (amount > 0.f)
	{
		if (m_cachedFamilyId != m_familyId)
		{
			m_cachedFamilyId     = m_familyId;
			m_cachedSgi          = generatorChunkData.shaderGroup->chooseShader (m_familyId);
			m_cachedFeatherClamp = generatorChunkData.shaderGroup->getFamilyFeatherClamp (m_familyId);
		}

		const float featherClamp = m_useFeatherClampOverride ? m_featherClampOverride : m_cachedFeatherClamp;

		FastRandomGenerator randomGenerator(CoordinateHash::hashTuple(worldX, worldZ));

		if (amount >= featherClamp)
		{
			ShaderGroup::Info sgi = m_cachedSgi;
			sgi.setChildChoice(randomGenerator.randomFloat());

			generatorChunkData.shaderMap->setData (x, z, sgi);
		}
	}
}

void AffectorShaderConstant::_legacyAffect (const float /*worldX*/, const float /*worldZ*/, const int x, const int z, const float amount, const TerrainGenerator::GeneratorChunkData& generatorChunkData) const
{
	if (amount > 0.f)
	{
		if (m_cachedFamilyId != m_familyId)
		{
			m_cachedFamilyId     = m_familyId;
			m_cachedSgi          = generatorChunkData.shaderGroup->chooseShader (m_familyId);
			m_cachedFeatherClamp = generatorChunkData.shaderGroup->getFamilyFeatherClamp (m_familyId);
		}

		const float featherClamp = m_useFeatherClampOverride ? m_featherClampOverride : m_cachedFeatherClamp;

		if (amount >= featherClamp)
		{
			ShaderGroup::Info sgi = m_cachedSgi;
			sgi.setChildChoice (generatorChunkData.m_legacyRandomGenerator->randomReal (0.0f, 1.0f));

			generatorChunkData.shaderMap->setData (x, z, sgi);
		}
	}
}

//-------------------------------------------------------------------

void AffectorShaderConstant::load (Iff& iff)
{
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
			DEBUG_FATAL (true, ("invalid AffectorShaderConstant version %s/%s", buffer, tagBuffer));
		}
		break;
	}
}

//-------------------------------------------------------------------

void AffectorShaderConstant::load_0000 (Iff& iff)
{
	iff.enterForm (TAG_0000);

		//-- load the base data
		LayerItem::load (iff);

		//-- load specific data
		iff.enterChunk (TAG_DATA);

			setFamilyId (iff.read_int32 ());

		iff.exitChunk (TAG_DATA);

	iff.exitForm (TAG_0000);
}

//-------------------------------------------------------------------

void AffectorShaderConstant::load_0001 (Iff& iff)
{
	iff.enterForm (TAG_0001);

		//-- load the base data
		LayerItem::load (iff);

		//-- load specific data
		iff.enterChunk (TAG_DATA);

			setFamilyId (iff.read_int32 ());
			setUseFeatherClampOverride (iff.read_int32 () != 0);
			setFeatherClampOverride (iff.read_float ());

		iff.exitChunk (TAG_DATA);

	iff.exitForm (TAG_0001);
}

//-------------------------------------------------------------------

void AffectorShaderConstant::save (Iff& iff) const
{
	iff.insertForm (TAG_0001);

		//-- save the base
		LayerItem::save (iff);

		//-- save specific data
		iff.insertChunk (TAG_DATA);

			iff.insertChunkData (getFamilyId ());
			iff.insertChunkData (getUseFeatherClampOverride () ? static_cast<int32> (1) : static_cast<int32> (0));
			iff.insertChunkData (getFeatherClampOverride ());

		iff.exitChunk (TAG_DATA);

	iff.exitForm (TAG_0001);
}

//-------------------------------------------------------------------
//
// AffectorShaderReplace
//
AffectorShaderReplace::AffectorShaderReplace () :
	TerrainGenerator::Affector (TAG_ASRP, TGAT_shaderReplace),
	m_cachedFamilyId (-1),
	m_cachedSgi (),
	m_cachedFeatherClamp (0.f),
	m_sourceFamilyId (0),
	m_destinationFamilyId (0),
	m_useFeatherClampOverride (false),
	m_featherClampOverride (1.f)
{
}

//-------------------------------------------------------------------

AffectorShaderReplace::~AffectorShaderReplace ()
{
}

//-------------------------------------------------------------------

void AffectorShaderReplace::setSourceFamilyId (const int sourceFamilyId)
{
	m_sourceFamilyId = sourceFamilyId;
}

//-------------------------------------------------------------------

void AffectorShaderReplace::setDestinationFamilyId (const int destinationFamilyId)
{
	m_destinationFamilyId = destinationFamilyId;
}

//-------------------------------------------------------------------

void AffectorShaderReplace::setUseFeatherClampOverride (const bool useFeatherClampOverride)
{
	m_useFeatherClampOverride = useFeatherClampOverride;
}

//-------------------------------------------------------------------

void AffectorShaderReplace::setFeatherClampOverride (const float featherClampOverride)
{
	m_featherClampOverride = featherClampOverride;
}

//-------------------------------------------------------------------

bool AffectorShaderReplace::affectsShader () const
{
	return true;
}

//-------------------------------------------------------------------

void AffectorShaderReplace::prepare ()
{
	m_cachedFamilyId = -1;
}

//-------------------------------------------------------------------

unsigned AffectorShaderReplace::getAffectedMaps() const
{
	return TGM_shader;
}

//-------------------------------------------------------------------

void AffectorShaderReplace::affect (const float /*worldX*/, const float /*worldZ*/, const int x, const int z, const float amount, const TerrainGenerator::GeneratorChunkData& generatorChunkData) const
{
	if (amount > 0.f)
	{
		const ShaderGroup::Info sgi = generatorChunkData.shaderMap->getData (x, z);

		if (sgi.getFamilyId () == m_sourceFamilyId)
		{
			if (m_cachedFamilyId != m_destinationFamilyId)
			{
				m_cachedFamilyId     = m_destinationFamilyId;
				m_cachedSgi          = generatorChunkData.shaderGroup->chooseShader (m_destinationFamilyId);
				m_cachedFeatherClamp = generatorChunkData.shaderGroup->getFamilyFeatherClamp (m_destinationFamilyId);
			}

			const float featherClamp = m_useFeatherClampOverride ? m_featherClampOverride : m_cachedFeatherClamp;

			if (amount >= featherClamp)
			{
				m_cachedSgi.setChildChoice (sgi.getChildChoice ());

				generatorChunkData.shaderMap->setData (x, z, m_cachedSgi);
			}
		}
	}
}

//-------------------------------------------------------------------

void AffectorShaderReplace::load (Iff& iff)
{
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
			DEBUG_FATAL (true, ("invalid AffectorShaderReplace version %s/%s", buffer, tagBuffer));
		}
		break;
	}
}

//-------------------------------------------------------------------

void AffectorShaderReplace::load_0000 (Iff& iff)
{
	iff.enterForm (TAG_0000);

		//-- load the base data
		LayerItem::load (iff);

		//-- load specific data
		iff.enterChunk (TAG_DATA);

			setSourceFamilyId (iff.read_int32 ());
			setDestinationFamilyId (iff.read_int32 ());

		iff.exitChunk (TAG_DATA);

	iff.exitForm (TAG_0000);
}

//-------------------------------------------------------------------

void AffectorShaderReplace::load_0001 (Iff& iff)
{
	iff.enterForm (TAG_0001);

		//-- load the base data
		LayerItem::load (iff);

		//-- load specific data
		iff.enterChunk (TAG_DATA);

			setSourceFamilyId (iff.read_int32 ());
			setDestinationFamilyId (iff.read_int32 ());
			setUseFeatherClampOverride (iff.read_int32 () != 0);
			setFeatherClampOverride (iff.read_float ());

		iff.exitChunk (TAG_DATA);

	iff.exitForm (TAG_0001);
}

//-------------------------------------------------------------------

void AffectorShaderReplace::save (Iff& iff) const
{
	iff.insertForm (TAG_0001);

		//-- save the base
		LayerItem::save (iff);

		//-- save specific data
		iff.insertChunk (TAG_DATA);

			iff.insertChunkData (getSourceFamilyId ());
			iff.insertChunkData (getDestinationFamilyId ());
			iff.insertChunkData (getUseFeatherClampOverride () ? static_cast<int32> (1) : static_cast<int32> (0));
			iff.insertChunkData (getFeatherClampOverride ());

		iff.exitChunk (TAG_DATA);

	iff.exitForm (TAG_0001);
}

//-------------------------------------------------------------------

