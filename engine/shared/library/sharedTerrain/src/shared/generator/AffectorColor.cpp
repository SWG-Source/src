//
// AffectorColor.cpp
// asommers 9-30-2000
//
// copyright 2000, verant interactive
//

//-------------------------------------------------------------------

#include "sharedTerrain/FirstSharedTerrain.h"
#include "sharedTerrain/AffectorColor.h"

#include "sharedFile/Iff.h"
#include "sharedFractal/MultiFractal.h"
#include "sharedFractal/MultiFractalReaderWriter.h"
#include "sharedImage/Image.h"
#include "sharedImage/ImageFormatList.h"
#include "sharedTerrain/Affector.h"
#include "sharedSynchronization/Mutex.h"

#include <algorithm>
#include <string>

//-------------------------------------------------------------------

static const PackedRgb computeColor (const PackedRgb& oldColor, const PackedRgb& desiredColor, const TerrainGeneratorOperation operation, const float amount)
{
	PackedRgb newColor = desiredColor;

	if (amount < 1.f)
	{
		newColor.r = static_cast<uint8> (desiredColor.r * amount);
		newColor.g = static_cast<uint8> (desiredColor.g * amount);
		newColor.b = static_cast<uint8> (desiredColor.b * amount);
	}

	switch (operation)
	{
	case TGO_add:
		{
			newColor.r = static_cast<uint8> (std::min (oldColor.r + newColor.r, 255));
			newColor.g = static_cast<uint8> (std::min (oldColor.g + newColor.g, 255));
			newColor.b = static_cast<uint8> (std::min (oldColor.b + newColor.b, 255));
		}
		break;

	case TGO_subtract:
		{
			newColor.r = static_cast<uint8> (std::max (oldColor.r - newColor.r, 0));
			newColor.g = static_cast<uint8> (std::max (oldColor.g - newColor.g, 0));
			newColor.b = static_cast<uint8> (std::max (oldColor.b - newColor.b, 0));
		}
		break;

	case TGO_multiply:
		{
			newColor.r = static_cast<uint8> (amount * (0.5f * desiredColor.r + 0.5f * oldColor.r) + (1.f - amount) * oldColor.r);
			newColor.g = static_cast<uint8> (amount * (0.5f * desiredColor.g + 0.5f * oldColor.g) + (1.f - amount) * oldColor.g);
			newColor.b = static_cast<uint8> (amount * (0.5f * desiredColor.b + 0.5f * oldColor.b) + (1.f - amount) * oldColor.b);
		}
		break;

	case TGO_replace:
	default:
		{
			newColor.r = static_cast<uint8> (amount * desiredColor.r + (1.f - amount) * oldColor.r);
			newColor.g = static_cast<uint8> (amount * desiredColor.g + (1.f - amount) * oldColor.g);
			newColor.b = static_cast<uint8> (amount * desiredColor.b + (1.f - amount) * oldColor.b);
		}
		break;

	case TGO_COUNT:
		FATAL (true, ("invalid operation"));
		break;
	}

	return newColor;
}

//-------------------------------------------------------------------
//
// AffectorColorConstant
//
AffectorColorConstant::AffectorColorConstant () :
	TerrainGenerator::Affector (TAG_ACCN, TGAT_colorConstant),
	operation (TGO_replace),
	color ()
{
}

//-------------------------------------------------------------------

AffectorColorConstant::~AffectorColorConstant ()
{
}

//-------------------------------------------------------------------

void AffectorColorConstant::setOperation (const TerrainGeneratorOperation newOperation)
{
	operation = newOperation;
}

//-------------------------------------------------------------------

void AffectorColorConstant::setColor (const PackedRgb& newColor)
{
	color = newColor;
}

//-------------------------------------------------------------------

unsigned AffectorColorConstant::getAffectedMaps() const
{
	return TGM_color;
}

//-------------------------------------------------------------------

void AffectorColorConstant::affect (const float /*worldX*/, const float /*worldZ*/, const int x, const int z, const float amount, const TerrainGenerator::GeneratorChunkData& generatorChunkData) const
{
	if (amount > 0.f)
	{
		const PackedRgb oldColor = generatorChunkData.colorMap->getData (x, z);
		const PackedRgb newColor = computeColor (oldColor, color, operation, amount);

		generatorChunkData.colorMap->setData (x, z, newColor);
	}
}

//-------------------------------------------------------------------

void AffectorColorConstant::load (Iff& iff)
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
			DEBUG_FATAL (true, ("invalid AffectorColorConstant version %s/%s", buffer, tagBuffer));
		}
		break;
	}
}

//-------------------------------------------------------------------

void AffectorColorConstant::load_0000 (Iff& iff)
{
	iff.enterForm (TAG_0000);

		//-- load the base data
		LayerItem::load (iff);

		//-- load specific data
		iff.enterChunk (TAG_DATA);

			int newOperation = iff.read_int32 ();
			DEBUG_FATAL (newOperation < 0 || newOperation >= TGO_COUNT, ("operation out of bounds for %s (%i)", getName (), newOperation));
			operation = static_cast<TerrainGeneratorOperation> (newOperation);

			color.r = iff.read_uint8 ();
			color.g = iff.read_uint8 ();
			color.b = iff.read_uint8 ();
			 
		iff.exitChunk (TAG_DATA);

	iff.exitForm (TAG_0000);
}

//-------------------------------------------------------------------

void AffectorColorConstant::save (Iff& iff) const
{
	iff.insertForm (TAG_0000);

		//-- save the base
		LayerItem::save (iff);

		//-- save specific data
		iff.insertChunk (TAG_DATA);

			iff.insertChunkData (static_cast<int32> (operation));
			iff.insertChunkData (color.r);
			iff.insertChunkData (color.g);
			iff.insertChunkData (color.b);

		iff.exitChunk (TAG_DATA);

	iff.exitForm (TAG_0000);
}

//-------------------------------------------------------------------
//
// AffectorColorRampHeight
//
AffectorColorRampHeight::AffectorColorRampHeight () :
	TerrainGenerator::Affector (TAG_ACRH, TGAT_colorRampHeight),
	image (0),
	operation (TGO_replace),
	lowHeight (0),
	highHeight (0),
	imageName (NON_NULL (new std::string))
{
}

//-------------------------------------------------------------------

AffectorColorRampHeight::~AffectorColorRampHeight ()
{
	if (image)
	{
		delete image;
		image = 0;
	}

	delete imageName;
	imageName = 0;
}

//-------------------------------------------------------------------

void AffectorColorRampHeight::setOperation (const TerrainGeneratorOperation newOperation)
{
	operation = newOperation;
}

//-------------------------------------------------------------------

void AffectorColorRampHeight::setLowHeight (const float newLowHeight)
{
	lowHeight = newLowHeight;
}

//-------------------------------------------------------------------

void AffectorColorRampHeight::setHighHeight (const float newHighHeight)
{
	highHeight = newHighHeight;
}

//-------------------------------------------------------------------

void AffectorColorRampHeight::setImage (const std::string& newImageName)
{
	*imageName = newImageName;

	delete image;

	image = ImageFormatList::loadImage (imageName->c_str ());
	if (image)
	{
		DEBUG_WARNING (image->getHeight () != 1, ("height != 1"));
		DEBUG_WARNING (! (image->getPixelFormat () == Image::PF_bgr_888 || image->getPixelFormat () == Image::PF_rgb_888), ("image is not in rgb or bgr format"));
	}
}

//-------------------------------------------------------------------

static const PackedRgb getPixel (const Image* image, const int x, const int y)
{
	PackedRgb result = PackedRgb::solidBlack;

	if (x >= 0 && x < image->getWidth () &&
		y >= 0 && y < image->getHeight ())
	{
		static Mutex mutex;

			mutex.enter ();

			const uint8* data = image->lockReadOnly ();

			data += y * image->getStride () + x * image->getBytesPerPixel ();

			if (image->getPixelFormat () == Image::PF_bgr_888)
			{
				result.b = *data++;
				result.g = *data++;
				result.r = *data++;
			}
			else
			{
				result.r = *data++;
				result.g = *data++;
				result.b = *data++;
			}

			image->unlock ();

		mutex.leave ();
	}

	return result;
}

//-------------------------------------------------------------------

unsigned AffectorColorRampHeight::getAffectedMaps() const
{
	return TGM_color;
}

//-------------------------------------------------------------------

void AffectorColorRampHeight::affect (const float /*worldX*/, const float /*worldZ*/, const int x, const int z, const float amount, const TerrainGenerator::GeneratorChunkData& generatorChunkData) const
{
	if (image && amount > 0.f)
	{
		const float height = generatorChunkData.heightMap->getData (x, z);
		if (WithinRangeInclusiveInclusive (lowHeight, height, highHeight))
		{
			const PackedRgb oldColor = generatorChunkData.colorMap->getData (x, z);
			const float t = (height - lowHeight) / (highHeight - lowHeight);
			const PackedRgb color = getPixel (image, static_cast<int> (t * (image->getWidth () - 1)), 0); 
			const PackedRgb newColor = computeColor (oldColor, color, operation, amount);

			generatorChunkData.colorMap->setData (x, z, newColor);
		}
	}
}

//-------------------------------------------------------------------

void AffectorColorRampHeight::load (Iff& iff)
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
			DEBUG_FATAL (true, ("invalid AffectorColorRampHeight version %s/%s", buffer, tagBuffer));
		}
		break;
	}
}

//-------------------------------------------------------------------

void AffectorColorRampHeight::load_0000 (Iff& iff)
{
	iff.enterForm (TAG_0000);

		//-- load the base data
		LayerItem::load (iff);

		//-- load specific data
		iff.enterChunk (TAG_DATA);

			int newOperation = iff.read_int32 ();
			DEBUG_FATAL (newOperation < 0 || newOperation >= TGO_COUNT, ("operation out of bounds for %s (%i)", getName (), newOperation));
			operation = static_cast<TerrainGeneratorOperation> (newOperation);

			lowHeight = iff.read_float ();
			highHeight = iff.read_float ();

			char* newImageName = iff.read_string ();
			setImage (newImageName);
			delete [] newImageName;

		iff.exitChunk (TAG_DATA);

	iff.exitForm (TAG_0000);
}

//-------------------------------------------------------------------

void AffectorColorRampHeight::save (Iff& iff) const
{
	iff.insertForm (TAG_0000);

		//-- save the base
		LayerItem::save (iff);

		//-- save specific data
		iff.insertChunk (TAG_DATA);

			iff.insertChunkData (static_cast<int32> (operation));
			iff.insertChunkData (lowHeight);
			iff.insertChunkData (highHeight);
			iff.insertChunkString (imageName->c_str ());

		iff.exitChunk (TAG_DATA);

	iff.exitForm (TAG_0000);
}

//-------------------------------------------------------------------
//
// AffectorColorRampFractal
//
AffectorColorRampFractal::AffectorColorRampFractal () :
	TerrainGenerator::Affector (TAG_ACRF, TGAT_colorRampFractal),
	m_multiFractal (0),
	m_cachedFamilyId (-1),
	image (0),
	m_familyId (0),
	operation (TGO_replace),
	imageName (NON_NULL (new std::string))
{
}

//-------------------------------------------------------------------

AffectorColorRampFractal::~AffectorColorRampFractal ()
{
	if (image)
	{
		delete image;
		image = 0;
	}

	delete imageName;
	imageName = 0;

	m_multiFractal = 0;
}

//-------------------------------------------------------------------

void AffectorColorRampFractal::setFamilyId (const int newFamilyId)
{
	m_familyId = newFamilyId;
}

//-------------------------------------------------------------------

void AffectorColorRampFractal::setOperation (const TerrainGeneratorOperation newOperation)
{
	operation = newOperation;
}

//-------------------------------------------------------------------

void AffectorColorRampFractal::setImage (const std::string& newImageName)
{
	*imageName = newImageName;

	delete image;

	image = ImageFormatList::loadImage (imageName->c_str ());
	if (image)
	{
		DEBUG_WARNING (image->getHeight () != 1, ("height != 1"));
		DEBUG_WARNING (! (image->getPixelFormat () == Image::PF_bgr_888 || image->getPixelFormat () == Image::PF_rgb_888), ("image is not in rgb or bgr format"));
	}
}

//-------------------------------------------------------------------

unsigned AffectorColorRampFractal::getAffectedMaps() const
{
	return TGM_color;
}

//-------------------------------------------------------------------

void AffectorColorRampFractal::affect (const float worldX, const float worldZ, const int x, const int z, const float amount, const TerrainGenerator::GeneratorChunkData& generatorChunkData) const
{
	if (image && amount > 0.f)
	{
		if (m_cachedFamilyId != m_familyId)
		{
			m_cachedFamilyId = m_familyId;
			m_multiFractal = generatorChunkData.fractalGroup->getFamilyMultiFractal (m_familyId);
		}

		NOT_NULL (m_multiFractal);

		const PackedRgb oldColor = generatorChunkData.colorMap->getData (x, z);
		const float t = m_multiFractal->getValueCache (worldX, worldZ, x, z);
		const PackedRgb color = getPixel (image, static_cast<int> (t * (image->getWidth () - 1)), 0); 
		const PackedRgb newColor = computeColor (oldColor, color, operation, amount);

		generatorChunkData.colorMap->setData (x, z, newColor);
	}
}

//-------------------------------------------------------------------

void AffectorColorRampFractal::load (Iff& iff, FractalGroup& fractalGroup)
{
	switch (iff.getCurrentName ())
	{
	case TAG_0000:
		load_0000 (iff, fractalGroup);
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
			DEBUG_FATAL (true, ("invalid AffectorColorRampFractal version %s/%s", buffer, tagBuffer));
		}
		break;
	}
}

//-------------------------------------------------------------------

void AffectorColorRampFractal::load_0000 (Iff& iff, FractalGroup& fractalGroup)
{
	iff.enterForm (TAG_0000);

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
				operation = static_cast<TerrainGeneratorOperation> (newOperation);

				char* newImageName = iff.read_string ();
				setImage (newImageName);
				delete [] newImageName;

			iff.exitChunk ();

		iff.exitForm (TAG_DATA);

	iff.exitForm (TAG_0000);
}

//-------------------------------------------------------------------

void AffectorColorRampFractal::load_0001 (Iff& iff)
{
	iff.enterForm (TAG_0001);

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

				char* newImageName = iff.read_string ();
				setImage (newImageName);
				delete [] newImageName;

			iff.exitChunk ();

		iff.exitForm (TAG_DATA);

	iff.exitForm (TAG_0001);
}

//-------------------------------------------------------------------

void AffectorColorRampFractal::save (Iff& iff) const
{
	iff.insertForm (TAG_0001);

		//-- save the base
		LayerItem::save (iff);

		//-- save specific data
		iff.insertForm (TAG_DATA);

			iff.insertChunk (TAG (P,A,R,M));

				iff.insertChunkData (getFamilyId ());
				iff.insertChunkData (static_cast<int32> (getOperation ()));
				iff.insertChunkString (getImageName ().c_str ());

			iff.exitChunk ();

		iff.exitForm ();

	iff.exitForm ();
}

//-------------------------------------------------------------------

