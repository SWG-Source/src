//===================================================================
//
// AffectorFloraDynamic.cpp
// asommers 9-30-2000
//
// copyright 2000, verant interactive
//
//===================================================================

#include "sharedTerrain/FirstSharedTerrain.h"
#include "sharedTerrain/AffectorFloraDynamic.h"

#include "sharedTerrain/Affector.h"
#include "sharedTerrain/CoordinateHash.h"
#include "sharedRandom/FastRandomGenerator.h"
#include "sharedFile/Iff.h"

//===================================================================
//
// AffectorFloraDynamic
//
AffectorFloraDynamic::AffectorFloraDynamic (const Tag newTag, const TerrainGeneratorAffectorType newType) :
	TerrainGenerator::Affector (newTag, newType),
	cachedFamilyId (-1),
	cachedRgi (),
	cachedDensity (0.f),
	familyId (0),
	operation (TGO_add),
	removeAll (false),
	densityOverride (false),
	densityOverrideDensity (1.f)
{
}

//-------------------------------------------------------------------

AffectorFloraDynamic::~AffectorFloraDynamic ()
{
}

//-------------------------------------------------------------------

void AffectorFloraDynamic::setFamilyId (const int newFamilyId)
{
	familyId = newFamilyId;
}

//-------------------------------------------------------------------

void AffectorFloraDynamic::setOperation (const TerrainGeneratorOperation newOperation)
{
	operation = newOperation;
}

//-------------------------------------------------------------------

void AffectorFloraDynamic::setRemoveAll (const bool newRemoveAll)
{
	removeAll = newRemoveAll;
}

//-------------------------------------------------------------------

void AffectorFloraDynamic::setDensityOverride (const bool newDensityOverride)
{
	densityOverride = newDensityOverride;
}

//-------------------------------------------------------------------

void AffectorFloraDynamic::setDensityOverrideDensity (const float newDensityOverrideDensity)
{
	densityOverrideDensity = newDensityOverrideDensity;
}

//-------------------------------------------------------------------

void AffectorFloraDynamic::prepare ()
{
	cachedFamilyId = -1;
}

//-------------------------------------------------------------------

void AffectorFloraDynamic::affect (const float worldX, const float worldZ, const int x, const int z, const float amount, const TerrainGenerator::GeneratorChunkData& generatorChunkData) const
{
	if (generatorChunkData.m_legacyRandomGenerator)
	{
		_legacyAffect(worldX, worldZ, x, z, amount, generatorChunkData);
		return;
	}
	if (amount > 0.f)
	{
		Array2d<RadialGroup::Info>* const floraMap = getFloraMap (generatorChunkData);

		if (removeAll)
		{
			//-- we want to remove all radial
			floraMap->setData (x, z, generatorChunkData.radialGroup->getDefaultRadial ());
		}
		else
		{
			DEBUG_FATAL (familyId == 0, ("familyId == 0 for %s", getName ()));

			if (cachedFamilyId != familyId)
			{
				cachedFamilyId = familyId;
				cachedRgi      = generatorChunkData.radialGroup->chooseRadial (familyId);
				cachedDensity  = generatorChunkData.radialGroup->getFamilyDensity (familyId);
			}

			//-- do we place flora here?
			const float density = densityOverride ? densityOverrideDensity : cachedDensity;

			FastRandomGenerator randomGenerator(CoordinateHash::hashTuple(worldX, worldZ));

			if (randomGenerator.randomFloat() <= amount * density)
			{
				RadialGroup::Info rgi = cachedRgi;

				if (rgi.getFamilyId() != 0)
				{
					switch (operation)
					{
					case TGO_add:
						{
					  		rgi.setChildChoice(randomGenerator.randomFloat());

							floraMap->setData (x, z, rgi);
						}
						break;

					case TGO_replace:
						{
							//-- replace is a remove of a specific type
							if (floraMap->getData(x, z).getFamilyId() == familyId)
								floraMap->setData (x, z, generatorChunkData.radialGroup->getDefaultRadial ());
						}
						break;

					case TGO_subtract:
					case TGO_multiply:
					case TGO_COUNT:
					default:
						DEBUG_FATAL (true, ("invalid operation"));
					}
				}
			}
		}
	}

}

void AffectorFloraDynamic::_legacyAffect (const float /*worldX*/, const float /*worldZ*/, const int x, const int z, const float amount, const TerrainGenerator::GeneratorChunkData& generatorChunkData) const
{
	if (amount > 0.f)
	{
		Array2d<RadialGroup::Info>* const floraMap = getFloraMap (generatorChunkData);

		if (removeAll)
		{
			//-- we want to remove all radial
			floraMap->setData (x, z, generatorChunkData.radialGroup->getDefaultRadial ());
		}
		else
		{
			DEBUG_FATAL (familyId == 0, ("familyId == 0 for %s", getName ()));

			if (cachedFamilyId != familyId)
			{
				cachedFamilyId = familyId;
				cachedRgi      = generatorChunkData.radialGroup->chooseRadial (familyId);
				cachedDensity  = generatorChunkData.radialGroup->getFamilyDensity (familyId);
			}

			//-- do we place flora here?
			const float density = densityOverride ? densityOverrideDensity : cachedDensity;

			if (generatorChunkData.m_legacyRandomGenerator->randomReal (0.f, 1.f) <= amount * density)
			{
				RadialGroup::Info rgi = cachedRgi;

				if (rgi.getFamilyId() != 0)
				{
					switch (operation)
					{
					case TGO_add:
						{
					  		rgi.setChildChoice(generatorChunkData.m_legacyRandomGenerator->randomReal(0.f, 1.f));

							floraMap->setData (x, z, rgi);
						}
						break;

					case TGO_replace:
						{
							//-- replace is a remove of a specific type
							if (floraMap->getData(x, z).getFamilyId() == familyId)
								floraMap->setData (x, z, generatorChunkData.radialGroup->getDefaultRadial ());
						}
						break;

					case TGO_subtract:
					case TGO_multiply:
					case TGO_COUNT:
					default:
						DEBUG_FATAL (true, ("invalid operation"));
					}
				}
			}
		}
	}
}

//-------------------------------------------------------------------

void AffectorFloraDynamic::load (Iff& iff)
{
	switch (iff.getCurrentName ())
	{
	case TAG_0000:
		load_0000 (iff);
		break;

	case TAG_0001:
		load_0001 (iff);
		break;

	case TAG_0002:
		load_0002 (iff);
		break;

	default:
		{
			char tagBuffer [5];
			ConvertTagToString (iff.getCurrentName (), tagBuffer);

			char buffer [128];
			iff.formatLocation (buffer, sizeof (buffer));
			DEBUG_FATAL (true, ("invalid AffectorFloraDynamic version %s/%s", buffer, tagBuffer));
		}
		break;
	}
}

//-------------------------------------------------------------------

void AffectorFloraDynamic::load_0000 (Iff& iff)
{
	iff.enterForm (TAG_0000);

		//-- load the base data
		LayerItem::load (iff);

		//-- load specific data
		iff.enterChunk (TAG_DATA);

			familyId = iff.read_int32 ();

		iff.exitChunk (TAG_DATA);

	iff.exitForm (TAG_0000);
}

//-------------------------------------------------------------------

void AffectorFloraDynamic::load_0001 (Iff& iff)
{
	iff.enterForm (TAG_0001);

		//-- load the base data
		LayerItem::load (iff);

		//-- load specific data
		iff.enterChunk (TAG_DATA);
			familyId = iff.read_int32 ();

			int newOperation = iff.read_int32 ();
			DEBUG_FATAL (newOperation < 0 || newOperation >= TGO_COUNT, ("operation out of bounds for %s", getName ()));
			operation = static_cast<TerrainGeneratorOperation> (newOperation);

			removeAll = iff.read_int32 () != 0;

		iff.exitChunk (TAG_DATA);

	iff.exitForm (TAG_0001);
}

//-------------------------------------------------------------------

void AffectorFloraDynamic::load_0002 (Iff& iff)
{
	iff.enterForm (TAG_0002);

		//-- load the base data
		LayerItem::load (iff);

		//-- load specific data
		iff.enterChunk (TAG_DATA);
			familyId = iff.read_int32 ();

			int newOperation = iff.read_int32 ();
			DEBUG_FATAL (newOperation < 0 || newOperation >= TGO_COUNT, ("operation out of bounds for %s", getName ()));
			operation = static_cast<TerrainGeneratorOperation> (newOperation);

			removeAll = iff.read_int32 () != 0;

			densityOverride        = iff.read_int32 () != 0;
			densityOverrideDensity = iff.read_float ();

		iff.exitChunk (TAG_DATA);

	iff.exitForm (TAG_0002);
}

//-------------------------------------------------------------------

void AffectorFloraDynamic::save (Iff& iff) const
{
	iff.insertForm (TAG_0002);

		//-- save the base
		LayerItem::save (iff);

		//-- save specific data
		iff.insertChunk (TAG_DATA);

			iff.insertChunkData (familyId);
			iff.insertChunkData (static_cast<int32> (operation));
			iff.insertChunkData (removeAll ? static_cast<int32> (1) : static_cast<int32> (0));
			iff.insertChunkData (densityOverride ? static_cast<int32> (1) : static_cast<int32> (0));
			iff.insertChunkData (densityOverrideDensity);

		iff.exitChunk (TAG_DATA);

	iff.exitForm (TAG_0002);
}

//===================================================================

AffectorFloraDynamicNearConstant::AffectorFloraDynamicNearConstant () :
	AffectorFloraDynamic (TAG_AFDN, TGAT_floraDynamicNearConstant)
{
}

//-------------------------------------------------------------------
	
AffectorFloraDynamicNearConstant::~AffectorFloraDynamicNearConstant ()
{
}

//-------------------------------------------------------------------

unsigned AffectorFloraDynamicNearConstant::getAffectedMaps() const
{
	return TGM_floraDynamicNear;
}

//===================================================================
//
// AffectorFloraFloraDynamicFarConstant
//

AffectorFloraDynamicFarConstant::AffectorFloraDynamicFarConstant () :
	AffectorFloraDynamic (TAG_AFDF, TGAT_floraDynamicFarConstant)
{
}

//-------------------------------------------------------------------
	
AffectorFloraDynamicFarConstant::~AffectorFloraDynamicFarConstant ()
{
}

//-------------------------------------------------------------------

unsigned AffectorFloraDynamicFarConstant::getAffectedMaps() const
{
	return TGM_floraDynamicFar;
}


//===================================================================
