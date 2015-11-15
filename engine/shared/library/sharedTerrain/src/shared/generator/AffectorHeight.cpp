//
// AffectorHeight.cpp
// asommers 9-30-2000
//
// copyright 2000, verant interactive
//

//-------------------------------------------------------------------

#include "sharedTerrain/FirstSharedTerrain.h"
#include "sharedTerrain/AffectorHeight.h"

#include "sharedFile/Iff.h"
#include "sharedFractal/MultiFractal.h"
#include "sharedFractal/MultiFractalReaderWriter.h"
#include "sharedTerrain/Affector.h"

//-------------------------------------------------------------------
//
// AffectorHeightConstant
//
AffectorHeightConstant::AffectorHeightConstant () :
	TerrainGenerator::Affector (TAG_AHCN, TGAT_heightConstant),
	operation (TGO_replace),
	height (0)
{
}

//-------------------------------------------------------------------

AffectorHeightConstant::~AffectorHeightConstant ()
{
}

//-------------------------------------------------------------------

void AffectorHeightConstant::setOperation (const TerrainGeneratorOperation newOperation)
{
	operation = newOperation;
}

//-------------------------------------------------------------------

void AffectorHeightConstant::setHeight (const float newHeight)
{
	height = newHeight;
}

//-------------------------------------------------------------------

unsigned AffectorHeightConstant::getAffectedMaps() const
{
	return TGM_height;
}

//-------------------------------------------------------------------

void AffectorHeightConstant::affect (const float /*worldX*/, const float /*worldZ*/, const int x, const int z, const float amount, const TerrainGenerator::GeneratorChunkData& generatorChunkData) const
{
	if (amount > 0.f)
	{
		float newHeight = 0.f;

		switch (operation)
		{
		case TGO_add:
			newHeight = generatorChunkData.heightMap->getData (x, z) + amount*height;
			break;

		case TGO_subtract:
			newHeight = generatorChunkData.heightMap->getData (x, z) - amount*height;
			break;

		case TGO_multiply:
			{
				const float oldHeight     = generatorChunkData.heightMap->getData (x, z);
				const float desiredHeight = generatorChunkData.heightMap->getData (x, z) * height;
				newHeight = linearInterpolate (oldHeight, desiredHeight, amount);
			}
			break;

		case TGO_replace:
		default:
			newHeight = amount * height + (1.f - amount) * generatorChunkData.heightMap->getData (x, z);
			break;

		case TGO_COUNT:
			FATAL (true, ("invalid operation"));
			break;
		}

		generatorChunkData.heightMap->setData (x, z, newHeight);
	}
}

//-------------------------------------------------------------------

bool AffectorHeightConstant::affectsHeight () const
{
	return true;
}

//-------------------------------------------------------------------

void AffectorHeightConstant::load (Iff& iff)
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
			DEBUG_FATAL (true, ("invalid AffectorHeightConstant version %s/%s", buffer, tagBuffer));
		}
		break;
	}
}

//-------------------------------------------------------------------

void AffectorHeightConstant::load_0000 (Iff& iff)
{
	iff.enterForm (TAG_0000);

		//-- load the base data
		LayerItem::load (iff);

		//-- load specific data
		iff.enterChunk (TAG_DATA);

			int newOperation = iff.read_int32 ();
			DEBUG_FATAL (newOperation < 0 || newOperation >= TGO_COUNT, ("operation out of bounds for %s", getName ()));
			operation = static_cast<TerrainGeneratorOperation> (newOperation);

			height    = iff.read_float ();

		iff.exitChunk (TAG_DATA);

	iff.exitForm (TAG_0000);
}

//-------------------------------------------------------------------

void AffectorHeightConstant::save (Iff& iff) const
{
	iff.insertForm (TAG_0000);

		//-- save the base
		LayerItem::save (iff);

		//-- save specific data
		iff.insertChunk (TAG_DATA);

			iff.insertChunkData (static_cast<int32> (operation));
			iff.insertChunkData (height);

		iff.exitChunk (TAG_DATA);

	iff.exitForm (TAG_0000);
}

//-------------------------------------------------------------------
//
// AffectorHeightFractal
//
AffectorHeightFractal::AffectorHeightFractal () :
	TerrainGenerator::Affector (TAG_AHFR, TGAT_heightFractal),
	m_multiFractal (0),
	m_cachedFamilyId (-1),
	m_familyId (0),
	m_scaleY (1),
	m_operation (TGO_replace)
{
}

AffectorHeightFractal::~AffectorHeightFractal ()
{
	m_multiFractal = 0;
}

//-------------------------------------------------------------------

void AffectorHeightFractal::setOperation (const TerrainGeneratorOperation newOperation)
{
	m_operation = newOperation;
}

//-------------------------------------------------------------------

void AffectorHeightFractal::setFamilyId (const int newFamilyId)
{
	m_familyId = newFamilyId;
}

//-------------------------------------------------------------------

void AffectorHeightFractal::setScaleY (const float newScale)
{
	m_scaleY = newScale;
}

//-------------------------------------------------------------------

unsigned AffectorHeightFractal::getAffectedMaps() const
{
	return TGM_height;
}

//-------------------------------------------------------------------

void AffectorHeightFractal::affect (const float worldX, const float worldZ, const int x, const int z, const float amount, const TerrainGenerator::GeneratorChunkData& generatorChunkData) const
{
	if (amount > 0.f)
	{
		if (m_cachedFamilyId != m_familyId)
		{
			m_cachedFamilyId = m_familyId;
			m_multiFractal   = generatorChunkData.fractalGroup->getFamilyMultiFractal (m_familyId);
		}

		NOT_NULL (m_multiFractal);

		const float fractalHeight = m_scaleY * m_multiFractal->getValueCache (worldX, worldZ, x, z);
		const float oldHeight     = generatorChunkData.heightMap->getData (x, z);
			
		float newHeight = oldHeight;

		switch (m_operation)
		{
		case TGO_add:
			newHeight += amount * fractalHeight;
			break;

		case TGO_subtract:
			newHeight -= amount * fractalHeight;
			break;

		case TGO_multiply:
			{
				const float desiredHeight = oldHeight * fractalHeight;

				newHeight = linearInterpolate (oldHeight, desiredHeight, amount);
			}
			break;

		case TGO_replace:
		default:
			newHeight = linearInterpolate (oldHeight, fractalHeight, amount);
			break;

		case TGO_COUNT:
			FATAL (true, ("invalid operation"));
			break;
		}

		generatorChunkData.heightMap->setData (x, z, newHeight);
	}
}

//-------------------------------------------------------------------

bool AffectorHeightFractal::affectsHeight () const
{
	return true;
}

//-------------------------------------------------------------------

void AffectorHeightFractal::load (Iff& iff, FractalGroup& fractalGroup)
{
	switch (iff.getCurrentName ())
	{
	case TAG_0000:
		load_0000 (iff, fractalGroup);
		break;

	case TAG_0001:
		load_0001 (iff, fractalGroup);
		break;

	case TAG_0002:
		load_0002 (iff, fractalGroup);
		break;

	case TAG_0003:
		load_0003 (iff);
		break;

	default:
		{
			char tagBuffer [5];
			ConvertTagToString (iff.getCurrentName (), tagBuffer);

			char buffer [128];
			iff.formatLocation (buffer, sizeof (buffer));
			DEBUG_FATAL (true, ("invalid AffectorHeightFractal version %s/%s", buffer, tagBuffer));
		}
		break;
	}
}

//-------------------------------------------------------------------

void AffectorHeightFractal::load_0000 (Iff& iff, FractalGroup& fractalGroup)
{
	iff.enterForm (TAG_0000);

		//-- load the base data
		LayerItem::load (iff);

		//-- load specific data
		iff.enterChunk (TAG_DATA);

			MultiFractal multiFractal;

			int newOperation = iff.read_int32 ();
			DEBUG_FATAL (newOperation < 0 || newOperation >= TGO_COUNT, ("operation out of bounds for %s (%i)", getName (), newOperation));
			setOperation (static_cast<TerrainGeneratorOperation> (newOperation));

			const int fractalType = iff.read_int32 ();
			DEBUG_FATAL (fractalType < 0 || fractalType >= MultiFractal::CR_COUNT, ("fractal type out of range (%i >= %i", fractalType, MultiFractal::CR_COUNT));
			multiFractal.setCombinationRule (static_cast<MultiFractal::CombinationRule> (fractalType));

			const uint32 seed = iff.read_uint32 ();
			multiFractal.setSeed (seed);

			const Vector scale = iff.read_floatVector ();
			multiFractal.setScale (scale.x, scale.z);

			setScaleY (scale.y);

			setFamilyId (fractalGroup.createFamily (&multiFractal, getName ()));

		iff.exitChunk (TAG_DATA);

	iff.exitForm (TAG_0000);
}

//-------------------------------------------------------------------

void AffectorHeightFractal::load_0001 (Iff& iff, FractalGroup& fractalGroup)
{
	iff.enterForm (TAG_0001);

		//-- load the base data
		LayerItem::load (iff);

		//-- load specific data
		iff.enterChunk (TAG_DATA);

			MultiFractal multiFractal;

			int newOperation = iff.read_int32 ();
			DEBUG_FATAL (newOperation < 0 || newOperation >= TGO_COUNT, ("operation out of bounds for %s (%i)", getName (), newOperation));
			setOperation (static_cast<TerrainGeneratorOperation> (newOperation));

			const int fractalType = iff.read_int32 ();
			DEBUG_FATAL (fractalType < 0 || fractalType >= MultiFractal::CR_COUNT, ("fractal type out of range (%i >= %i", fractalType, MultiFractal::CR_COUNT));
			multiFractal.setCombinationRule (static_cast<MultiFractal::CombinationRule> (fractalType));

			multiFractal.setNumberOfOctaves (iff.read_int32 ());
			multiFractal.setFrequency (iff.read_float ());

			Vector scale;

			const int n = iff.read_int32 ();
			int i;
			for (i = 0; i < n; i++)
			{
				const int unused = iff.read_int32 ();
				UNREF (unused);
				scale.x = iff.read_float ();
				scale.z = iff.read_float ();
			}

			const uint32 seed = iff.read_uint32 ();
			multiFractal.setSeed (seed);

			for (i = 0; i < multiFractal.getNumberOfOctaves (); i++)
			{
				const int dummy1 = iff.read_int32 ();
				UNREF (dummy1);

				const int dummy2 = iff.read_int32 ();
				UNREF (dummy2);
			}
			
			setScaleY (iff.read_float ());

			multiFractal.setScale (scale.x, scale.z);

			setFamilyId (fractalGroup.createFamily (&multiFractal, getName ()));

		iff.exitChunk (TAG_DATA);

	iff.exitForm (TAG_0001);
}

//-------------------------------------------------------------------

void AffectorHeightFractal::load_0002 (Iff& iff, FractalGroup& fractalGroup)
{
	iff.enterForm (TAG_0002);

		//-- load the base data
		LayerItem::load (iff);

		//-- load specific data
		iff.enterForm (TAG_DATA);

			//-- load the fractal settings
			MultiFractal multiFractal;
			MultiFractalReaderWriter::load (iff, multiFractal);
			setFamilyId (fractalGroup.createFamily (&multiFractal, getName ()));

			//-- load parameters
			iff.enterChunk (TAG_PARM);

				int newOperation = iff.read_int32 ();
				DEBUG_FATAL (newOperation < 0 || newOperation >= TGO_COUNT, ("operation out of bounds for %s (%i)", getName (), newOperation));
				setOperation (static_cast<TerrainGeneratorOperation> (newOperation));

				setScaleY (iff.read_float ());

			iff.exitChunk ();

		iff.exitForm (TAG_DATA);

	iff.exitForm (TAG_0002);
}

//-------------------------------------------------------------------

void AffectorHeightFractal::load_0003 (Iff& iff)
{
	iff.enterForm (TAG_0003);

		//-- load the base data
		LayerItem::load (iff);

		//-- load specific data
		iff.enterForm (TAG_DATA);

			//-- load parameters
			iff.enterChunk (TAG_PARM);

				setFamilyId (iff.read_int32 ());

				int newOperation = iff.read_int32 ();
				DEBUG_FATAL (newOperation < 0 || newOperation >= TGO_COUNT, ("operation out of bounds for %s (%i)", getName (), newOperation));
				setOperation (static_cast<TerrainGeneratorOperation> (newOperation));

				setScaleY (iff.read_float ());

			iff.exitChunk ();

		iff.exitForm (TAG_DATA);

	iff.exitForm (TAG_0003);
}

//-------------------------------------------------------------------

void AffectorHeightFractal::save (Iff& iff) const
{
	iff.insertForm (TAG_0003);

		//-- save the base
		LayerItem::save (iff);

		//-- save specific data
		iff.insertForm (TAG_DATA);

			iff.insertChunk (TAG (P,A,R,M));

				iff.insertChunkData (getFamilyId ());
				iff.insertChunkData (static_cast<int32> (getOperation ()));
				iff.insertChunkData (getScaleY ());

			iff.exitChunk ();

		iff.exitForm ();

	iff.exitForm ();
}

//-------------------------------------------------------------------
//
// AffectorHeightTerrace
//
AffectorHeightTerrace::AffectorHeightTerrace ()  :
	TerrainGenerator::Affector (TAG_AHTR, TGAT_heightTerrace),
	height (20.0f),
	fraction (0.25f)
{
}

//-------------------------------------------------------------------

AffectorHeightTerrace::~AffectorHeightTerrace ()
{
}

//-------------------------------------------------------------------

unsigned AffectorHeightTerrace::getAffectedMaps() const
{
	return TGM_height;
}

//-------------------------------------------------------------------

void AffectorHeightTerrace::affect (const float /*worldX*/, const float /*worldZ*/, const int x, const int z, const float amount, const TerrainGenerator::GeneratorChunkData& generatorChunkData) const
{
	if (amount > 0.f && height > 0.f)
	{
		const float terraceHeight = height;

		if (terraceHeight > 0.f)
		{
			const float originalHeight = generatorChunkData.heightMap->getData (x, z);
			const float lowHeight      = originalHeight - ((originalHeight < 0) ? (terraceHeight + fmodf (originalHeight, terraceHeight)) : fmodf (originalHeight, terraceHeight));
			const float midHeight      = lowHeight + terraceHeight * fraction;
			const float highHeight     = lowHeight + terraceHeight;

			float newHeight = lowHeight;

			if (originalHeight > midHeight)
			{
				const float t = (originalHeight - midHeight) / (highHeight - midHeight);

				newHeight = linearInterpolate (lowHeight, highHeight, t);
			}

			generatorChunkData.heightMap->setData (x, z, linearInterpolate (originalHeight, newHeight, amount));
		}
	}
}

//-------------------------------------------------------------------

bool AffectorHeightTerrace::affectsHeight () const
{
	return true;
}

//-------------------------------------------------------------------

void AffectorHeightTerrace::load (Iff& iff)
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
			DEBUG_FATAL (true, ("invalid AffectorHeightTerrace version %s/%s", buffer, tagBuffer));
		}
		break;
	}
}

//-------------------------------------------------------------------

void AffectorHeightTerrace::load_0000 (Iff& iff)
{
	iff.enterForm (TAG_0000);

		//-- load the base data
		LayerItem::load (iff);

		//-- load specific data
		iff.enterChunk (TAG_DATA);
			fraction = iff.read_float ();
			height   = iff.read_float ();
		iff.exitChunk (TAG_DATA);

	iff.exitForm (TAG_0000);
}

//-------------------------------------------------------------------

void AffectorHeightTerrace::load_0001 (Iff& iff)
{
	iff.enterForm (TAG_0001);

		//-- load the base data
		LayerItem::load (iff);

		//-- load specific data
		iff.enterChunk (TAG_DATA);
			fraction     = iff.read_float ();
			height       = iff.read_float ();
		iff.exitChunk (TAG_DATA, true);

	iff.exitForm (TAG_0001);
}

//-------------------------------------------------------------------

void AffectorHeightTerrace::load_0002 (Iff& iff)
{
	iff.enterForm (TAG_0002);

		//-- load the base data
		LayerItem::load (iff);

		//-- load specific data
		iff.enterChunk (TAG_DATA);
			fraction     = iff.read_float ();
			height       = iff.read_float ();
		iff.exitChunk (TAG_DATA, true);

	iff.exitForm (TAG_0002);
}

//-------------------------------------------------------------------

void AffectorHeightTerrace::load_0003 (Iff& iff)
{
	iff.enterForm (TAG_0003);

		//-- load the base data
		LayerItem::load (iff);

		//-- load specific data
		iff.enterForm (TAG_DATA);

			//-- load the fractal settings
			{
				MultiFractal multiFractal;
				MultiFractalReaderWriter::load (iff, multiFractal);
			}

			//-- load parameters
			iff.enterChunk (TAG_PARM);

				fraction           = iff.read_float ();
				height             = iff.read_float ();
				const int unused   = iff.read_int32 ();
				UNREF (unused);

			iff.exitChunk ();

		iff.exitForm (TAG_DATA);

	iff.exitForm (TAG_0003);
}

//-------------------------------------------------------------------

void AffectorHeightTerrace::load_0004 (Iff& iff)
{
	iff.enterForm (TAG_0004);

		//-- load the base data
		LayerItem::load (iff);

		//-- load specific data
		iff.enterChunk (TAG_DATA);
			fraction = iff.read_float ();
			height   = iff.read_float ();
		iff.exitChunk (TAG_DATA);

	iff.exitForm (TAG_0004);
}

//-------------------------------------------------------------------

void AffectorHeightTerrace::save (Iff& iff) const
{
	iff.insertForm (TAG_0004);

		//-- save the base
		LayerItem::save (iff);

		//-- save specific data
		iff.insertChunk (TAG_DATA);
			iff.insertChunkData (fraction);
			iff.insertChunkData (height);
		iff.exitChunk ();

	iff.exitForm (TAG_0004);
}

//-------------------------------------------------------------------

void AffectorHeightTerrace::setHeight (const float newHeight)
{
	height = newHeight;
}

//-------------------------------------------------------------------

void AffectorHeightTerrace::setFraction (const float newFraction)
{
	fraction = newFraction;
}

//-------------------------------------------------------------------

