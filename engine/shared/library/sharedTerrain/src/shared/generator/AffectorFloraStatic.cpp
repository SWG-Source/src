//===================================================================
//
// AffectorFloraStatic.cpp
// asommers 9-30-2000
//
// copyright 2000, verant interactive
//
//===================================================================

#include "sharedTerrain/FirstSharedTerrain.h"
#include "sharedTerrain/AffectorFloraStatic.h"

#include "sharedTerrain/Affector.h"
#include "sharedTerrain/CoordinateHash.h"
#include "sharedRandom/FastRandomGenerator.h"
#include "sharedFile/Iff.h"

//===================================================================
//
// AffectorFloraStatic
//
AffectorFloraStatic::AffectorFloraStatic (const Tag newTag, const TerrainGeneratorAffectorType newType) :
	TerrainGenerator::Affector (newTag, newType),
	cachedFamilyId (-1),
	cachedFgi (),
	cachedDensity (0.f),
	familyId (0),
	operation (TGO_add),
	removeAll (false),
	densityOverride (false),
	densityOverrideDensity (1.f)
{
}

//-------------------------------------------------------------------

AffectorFloraStatic::~AffectorFloraStatic ()
{
}

//-------------------------------------------------------------------

void AffectorFloraStatic::setFamilyId (const int newFamilyId)
{
	familyId = newFamilyId;
}

//-------------------------------------------------------------------

void AffectorFloraStatic::setOperation (const TerrainGeneratorOperation newOperation)
{
	operation = newOperation;
}

//-------------------------------------------------------------------

void AffectorFloraStatic::setRemoveAll (const bool newRemoveAll)
{
	removeAll = newRemoveAll;
}

//-------------------------------------------------------------------

void AffectorFloraStatic::setDensityOverride (const bool newDensityOverride)
{
	densityOverride = newDensityOverride;
}

//-------------------------------------------------------------------

void AffectorFloraStatic::setDensityOverrideDensity (const float newDensityOverrideDensity)
{
	densityOverrideDensity = newDensityOverrideDensity;
}

//-------------------------------------------------------------------

void AffectorFloraStatic::prepare ()
{
	cachedFamilyId = -1;
}

//-------------------------------------------------------------------

unsigned AffectorFloraStaticCollidableConstant::getAffectedMaps() const
{
	return TGM_floraStaticCollidable;
}

//-------------------------------------------------------------------

unsigned AffectorFloraStaticNonCollidableConstant::getAffectedMaps() const
{
	return TGM_floraStaticNonCollidable;
}

//-------------------------------------------------------------------

void AffectorFloraStatic::affect (const float worldX, const float worldZ, const int x, const int z, const float amount, const TerrainGenerator::GeneratorChunkData& generatorChunkData) const
{
	if (generatorChunkData.m_legacyRandomGenerator)
	{
		_legacyAffect(worldX, worldZ, x, z, amount, generatorChunkData);
		return;
	}
	if (amount > 0.f)
	{
		Array2d<FloraGroup::Info>* const floraMap = getFloraMap (generatorChunkData);

		if (removeAll)
		{
			//-- we want to remove all flora
			floraMap->setData (x, z, generatorChunkData.floraGroup->getDefaultFlora ());
		}
		else
		{
			DEBUG_FATAL (familyId == 0, ("familyId == 0 for %s", getName ()));

			if (cachedFamilyId != familyId)
			{
				cachedFamilyId = familyId;
				cachedFgi      = generatorChunkData.floraGroup->chooseFlora (familyId);
				cachedDensity  = generatorChunkData.floraGroup->getFamilyDensity (familyId);
			}

			//-- do we place flora here?
			const float density = densityOverride ? densityOverrideDensity : cachedDensity;

			FastRandomGenerator randomGenerator(CoordinateHash::hashTuple(worldX, worldZ));

			float rf = randomGenerator.randomFloat();
			if (rf <= amount * density)
			{
				FloraGroup::Info fgi = cachedFgi;

				if (fgi.getFamilyId () != 0)
				{
					switch (operation)
					{
					case TGO_add:
						{
							rf = randomGenerator.randomFloat();
				  			fgi.setChildChoice(rf);

							floraMap->setData (x, z, fgi);
						}
						break;

					case TGO_replace:
						{
							//-- replace is a remove of a specific type
							if (floraMap->getData (x, z).getFamilyId () == familyId)
								floraMap->setData (x, z, generatorChunkData.floraGroup->getDefaultFlora ());
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

void AffectorFloraStatic::_legacyAffect(float /*worldX*/, float /*worldZ*/, int x, int z, float amount, const TerrainGenerator::GeneratorChunkData& generatorChunkData) const
{
	if (amount > 0.f)
	{
		Array2d<FloraGroup::Info>* const floraMap = getFloraMap (generatorChunkData);

		if (removeAll)
		{
			//-- we want to remove all flora
			floraMap->setData (x, z, generatorChunkData.floraGroup->getDefaultFlora ());
		}
		else
		{
			DEBUG_FATAL (familyId == 0, ("familyId == 0 for %s", getName ()));

			if (cachedFamilyId != familyId)
			{
				cachedFamilyId = familyId;
				cachedFgi      = generatorChunkData.floraGroup->chooseFlora (familyId);
				cachedDensity  = generatorChunkData.floraGroup->getFamilyDensity (familyId);
			}

			//-- do we place flora here?
			const float density = densityOverride ? densityOverrideDensity : cachedDensity;

			if (generatorChunkData.m_legacyRandomGenerator->randomReal (0.f, 1.f) <= amount * density)
			{
				FloraGroup::Info fgi = cachedFgi;

				if (fgi.getFamilyId () != 0)
				{
					switch (operation)
					{
					case TGO_add:
						{
				  			fgi.setChildChoice (generatorChunkData.m_legacyRandomGenerator->randomReal (0.0f, 1.0f));

							floraMap->setData (x, z, fgi);
						}
						break;

					case TGO_replace:
						{
							//-- replace is a remove of a specific type
							if (floraMap->getData (x, z).getFamilyId () == familyId)
								floraMap->setData (x, z, generatorChunkData.floraGroup->getDefaultFlora ());
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

void AffectorFloraStatic::load (Iff& iff)
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
	
	case TAG_0003:
		load_0003 (iff);
		break;

	case TAG_0004:
		load_0004 (iff);
		break;

	default:
		{
			char tagBuffer [5];
			ConvertTagToString (iff.getCurrentName (), tagBuffer);

			char buffer [128];
			iff.formatLocation (buffer, sizeof (buffer));
			DEBUG_FATAL (true, ("invalid AffectorFloraStatic version %s/%s", buffer, tagBuffer));
		}
		break;
	}
}

//-------------------------------------------------------------------

void AffectorFloraStatic::load_0000 (Iff& iff)
{
	iff.enterForm (TAG_0000);

		//-- load the base data
		LayerItem::load (iff);

		//-- load specific data
		iff.enterChunk (TAG_DATA);
			operation          = TGO_replace;
		iff.exitChunk (TAG_DATA);

	iff.exitForm (TAG_0000);
}

//-------------------------------------------------------------------

void AffectorFloraStatic::load_0001 (Iff& iff)
{
	iff.enterForm (TAG_0001);

		//-- load the base data
		LayerItem::load (iff);

		//-- load specific data
		iff.enterChunk (TAG_DATA);
			familyId  = iff.read_int32 ();
			operation = TGO_replace;
		iff.exitChunk (TAG_DATA);

	iff.exitForm (TAG_0001);
}

//-------------------------------------------------------------------

void AffectorFloraStatic::load_0002 (Iff& iff)
{
	iff.enterForm (TAG_0002);

		//-- load the base data
		LayerItem::load (iff);

		//-- load specific data
		iff.enterChunk (TAG_DATA);
			familyId = iff.read_int32 ();

			int newOperation = iff.read_int32 ();
			DEBUG_FATAL (newOperation < 0 || newOperation >= static_cast<int> (TGO_COUNT), ("operation out of bounds for %s", getName ()));
			operation = static_cast<TerrainGeneratorOperation> (newOperation);

		iff.exitChunk (TAG_DATA);

	iff.exitForm (TAG_0002);
}

//-------------------------------------------------------------------

void AffectorFloraStatic::load_0003 (Iff& iff)
{
	iff.enterForm (TAG_0003);

		//-- load the base data
		LayerItem::load (iff);

		//-- load specific data
		iff.enterChunk (TAG_DATA);
			familyId = iff.read_int32 ();

			int newOperation = iff.read_int32 ();
			DEBUG_FATAL (newOperation < 0 || newOperation >= static_cast<int> (TGO_COUNT), ("operation out of bounds for %s", getName ()));
			operation = static_cast<TerrainGeneratorOperation> (newOperation);

			removeAll = iff.read_int32 () != 0;

		iff.exitChunk (TAG_DATA);

	iff.exitForm (TAG_0003);
}

//-------------------------------------------------------------------

void AffectorFloraStatic::load_0004 (Iff& iff)
{
	iff.enterForm (TAG_0004);

		//-- load the base data
		LayerItem::load (iff);

		//-- load specific data
		iff.enterChunk (TAG_DATA);
			familyId = iff.read_int32 ();

			int newOperation = iff.read_int32 ();
			DEBUG_FATAL (newOperation < 0 || newOperation >= static_cast<int> (TGO_COUNT), ("operation out of bounds for %s", getName ()));
			operation = static_cast<TerrainGeneratorOperation> (newOperation);

			removeAll = iff.read_int32 () != 0;

			densityOverride        = iff.read_int32 () != 0;
			densityOverrideDensity = iff.read_float ();

		iff.exitChunk (TAG_DATA);

	iff.exitForm (TAG_0004);
}

//-------------------------------------------------------------------

void AffectorFloraStatic::save (Iff& iff) const
{
	iff.insertForm (TAG_0004);

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

	iff.exitForm (TAG_0004);
}

//===================================================================
//
// AffectorFloraStaticCollidableConstant
//

AffectorFloraStaticCollidableConstant::AffectorFloraStaticCollidableConstant () :
	AffectorFloraStatic (TAG_AFSC, TGAT_floraStaticCollidableConstant)
{
}

//-------------------------------------------------------------------
	
AffectorFloraStaticCollidableConstant::~AffectorFloraStaticCollidableConstant ()
{
}

//===================================================================
//
// AffectorFloraStaticNonCollidableConstant
//

AffectorFloraStaticNonCollidableConstant::AffectorFloraStaticNonCollidableConstant () :
	AffectorFloraStatic (TAG_AFSN, TGAT_floraStaticNonCollidableConstant)
{
}

//-------------------------------------------------------------------
	
AffectorFloraStaticNonCollidableConstant::~AffectorFloraStaticNonCollidableConstant ()
{
}

//===================================================================

