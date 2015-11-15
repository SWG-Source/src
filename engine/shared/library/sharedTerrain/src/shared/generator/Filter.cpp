//
// Filter.cpp
// asommers 9-30-2000
//
// copyright 2000, verant interactive
//

//-------------------------------------------------------------------

#include "sharedTerrain/FirstSharedTerrain.h"
#include "sharedTerrain/Filter.h"

#include "sharedFile/Iff.h"
#include "sharedFractal/MultiFractal.h"
#include "sharedFractal/MultiFractalReaderWriter.h"
#include "sharedImage/Image.h"

#include <algorithm>

//-------------------------------------------------------------------

static const float ooPI_TIMES_2 = 1.0f / PI_TIMES_2;

//-------------------------------------------------------------------

template<class T>
inline bool isWithinRange (const T& minValue, const T& value, const T& maxValue)
{
	return value >= minValue && value <= maxValue;
}

//-------------------------------------------------------------------

inline float computeFeatheredInterpolant (const float minimum, const float value, const float maximum, const float featherIn)
{
	if (!WithinRangeExclusiveExclusive (minimum, value, maximum))
	{
		return 0.f;
	}

	const float feather = featherIn * (maximum - minimum) * 0.5f;

	if (value < minimum + feather)
	{
		return (value - minimum) / feather;
	}
	else
		if (value > maximum - feather)
		{
			return (maximum - value) / feather;
		}
		
	return 1.f;
}

//-------------------------------------------------------------------
//
// FilterHeight
//
FilterHeight::FilterHeight () :
	TerrainGenerator::Filter (TAG_FHGT, TGFT_height),
	lowHeight (0),
	highHeight (0)
{
}

//-------------------------------------------------------------------

FilterHeight::~FilterHeight ()
{
}

//-------------------------------------------------------------------

float FilterHeight::isWithin (const float /*worldX*/, const float /*worldZ*/, const int x, const int z, const TerrainGenerator::GeneratorChunkData& generatorChunkData) const
{
	const float height = generatorChunkData.heightMap->getData (x, z);

	return computeFeatheredInterpolant (lowHeight, height, highHeight, getFeatherDistance ());
}

//-------------------------------------------------------------------

void FilterHeight::load (Iff& iff)
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
			DEBUG_FATAL (true, ("unknown layer type %s/%s", buffer, tagBuffer));
		}
		break;
	}
}

//-------------------------------------------------------------------

void FilterHeight::load_0000 (Iff& iff)
{
	iff.enterForm (TAG_0000);

		//-- load the base data
		LayerItem::load (iff);

		//-- load specific data
		iff.enterChunk (TAG_DATA);
			
			lowHeight  = iff.read_float ();
			highHeight = iff.read_float ();

		iff.exitChunk (TAG_DATA);

	iff.exitForm (TAG_0000);
}

//-------------------------------------------------------------------

void FilterHeight::load_0001 (Iff& iff)
{
	iff.enterForm (TAG_0001);

		//-- load the base data
		LayerItem::load (iff);

		//-- load specific data
		iff.enterChunk (TAG_DATA);
			
			float unused = iff.read_float ();
			UNREF (unused);

			lowHeight  = iff.read_float ();
			highHeight = iff.read_float ();

		iff.exitChunk (TAG_DATA);

	iff.exitForm (TAG_0001);
}

//-------------------------------------------------------------------

void FilterHeight::load_0002 (Iff& iff)
{
	iff.enterForm (TAG_0002);

		//-- load the base data
		LayerItem::load (iff);

		//-- load specific data
		iff.enterChunk (TAG_DATA);
			
			lowHeight  = iff.read_float ();
			highHeight = iff.read_float ();

			setFeatherFunction (static_cast<TerrainGeneratorFeatherFunction> (iff.read_int32 ()));

			const float newFeatherDistance = clamp (0.f, iff.read_float (), 1.f);
			setFeatherDistance (newFeatherDistance);

		iff.exitChunk (TAG_DATA);

	iff.exitForm (TAG_0002);
}

//-------------------------------------------------------------------

void FilterHeight::save (Iff& iff) const
{
	iff.insertForm (TAG_0002);

		//-- save the base
		LayerItem::save (iff);

		//-- save specific data
		iff.insertChunk (TAG_DATA);

			iff.insertChunkData (lowHeight);
			iff.insertChunkData (highHeight);

			iff.insertChunkData (static_cast<int32> (getFeatherFunction ()));
			iff.insertChunkData (getFeatherDistance ());

		iff.exitChunk (TAG_DATA);

	iff.exitForm (TAG_0002);
}

//-------------------------------------------------------------------

void FilterHeight::setLowHeight (float newLowHeight)
{
	lowHeight = newLowHeight;
}

//-------------------------------------------------------------------

void FilterHeight::setHighHeight (float newHighHeight)
{
	highHeight = newHighHeight;
}

//-------------------------------------------------------------------
//
// FilterFractal
//
FilterFractal::FilterFractal () :
	TerrainGenerator::Filter (TAG_FFRA, TGFT_fractal),
	m_multiFractal (0),
	m_cachedFamilyId (-1),
	m_familyId (0),
	m_scaleY (1),
	m_lowFractalLimit (0),
	m_highFractalLimit (0)
{
}

//-------------------------------------------------------------------

FilterFractal::~FilterFractal ()
{
	m_multiFractal = 0;
}

//-------------------------------------------------------------------

float FilterFractal::isWithin (const float worldX, const float worldZ, const int x, const int z, const TerrainGenerator::GeneratorChunkData& generatorChunkData) const
{
	if (m_cachedFamilyId != m_familyId)
	{
		m_cachedFamilyId = m_familyId;
		m_multiFractal   = generatorChunkData.fractalGroup->getFamilyMultiFractal (m_familyId);
	}

	NOT_NULL (m_multiFractal);
	const float fractalHeight = m_scaleY * m_multiFractal->getValueCache (worldX, worldZ, x, z);

	return computeFeatheredInterpolant (m_lowFractalLimit, fractalHeight, m_highFractalLimit, getFeatherDistance ());
}

//-------------------------------------------------------------------

void FilterFractal::load (Iff& iff, FractalGroup& fractalGroup)
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
		load_0003 (iff, fractalGroup);
		break;

	case TAG_0004:
		load_0004 (iff, fractalGroup);
		break;

	case TAG_0005:
		load_0005 (iff);
		break;

	default:
		{
			char tagBuffer [5];
			ConvertTagToString (iff.getCurrentName (), tagBuffer);

			char buffer [128];
			iff.formatLocation (buffer, sizeof (buffer));
			DEBUG_FATAL (true, ("unknown layer type %s/%s", buffer, tagBuffer));
		}
		break;
	}
}

//-------------------------------------------------------------------

void FilterFractal::load_0000 (Iff& iff, FractalGroup& fractalGroup)
{
	iff.enterForm (TAG_0000);

		//-- load the base data
		LayerItem::load (iff);

		//-- load specific data
		iff.enterChunk (TAG_DATA);

			MultiFractal multiFractal;

			const int fractalType  = iff.read_int32 ();
			DEBUG_FATAL (fractalType < 0 || fractalType >= MultiFractal::CR_COUNT, ("fractal type out of range (%i >= %i", fractalType, MultiFractal::CR_COUNT));
			multiFractal.setCombinationRule (static_cast<MultiFractal::CombinationRule> (fractalType));

			const uint32 seed  = iff.read_uint32 ();
			multiFractal.setSeed (seed);

			const Vector scale = iff.read_floatVector ();
			multiFractal.setScale (scale.x, scale.z);
			setScaleY (scale.y);

			setLowFractalLimit (iff.read_float ());
			setHighFractalLimit (iff.read_float ());
			setFamilyId (fractalGroup.createFamily (&multiFractal, getName ()));

		iff.exitChunk (TAG_DATA);

	iff.exitForm (TAG_0000);
}

//-------------------------------------------------------------------

void FilterFractal::load_0001 (Iff& iff, FractalGroup& fractalGroup)
{
	iff.enterForm (TAG_0001);

		//-- load the base data
		LayerItem::load (iff);

		//-- load specific data
		iff.enterChunk (TAG_DATA);

			MultiFractal multiFractal;

			setLowFractalLimit (iff.read_float ());
			setHighFractalLimit (iff.read_float ());

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

void FilterFractal::load_0002 (Iff& iff, FractalGroup& fractalGroup)
{
	iff.enterForm (TAG_0002);

		//-- load the base data
		LayerItem::load (iff);

		//-- load specific data
		iff.enterChunk (TAG_DATA);

			MultiFractal multiFractal;

			{
				float unused = iff.read_float ();
				UNREF (unused);
			}

			setLowFractalLimit (iff.read_float ());
			setHighFractalLimit (iff.read_float ());

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

	iff.exitForm (TAG_0002);
}

//-------------------------------------------------------------------

void FilterFractal::load_0003 (Iff& iff, FractalGroup& fractalGroup)
{
	iff.enterForm (TAG_0003);

		//-- load the base data
		LayerItem::load (iff);

		//-- load specific data
		iff.enterChunk (TAG_DATA);

			MultiFractal multiFractal;

			setFeatherFunction (static_cast<TerrainGeneratorFeatherFunction> (iff.read_int32 ()));

			const float newFeatherDistance = clamp (0.f, iff.read_float (), 1.f);
			setFeatherDistance (newFeatherDistance);

			setLowFractalLimit (iff.read_float ());
			setHighFractalLimit (iff.read_float ());

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

	iff.exitForm (TAG_0003);
}

//-------------------------------------------------------------------

void FilterFractal::load_0004 (Iff& iff, FractalGroup& fractalGroup)
{
	iff.enterForm (TAG_0004);

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

				setFeatherFunction (static_cast<TerrainGeneratorFeatherFunction> (iff.read_int32 ()));
				setFeatherDistance (clamp (0.f, iff.read_float (), 1.f));
				setLowFractalLimit (iff.read_float ());
				setHighFractalLimit (iff.read_float ());
				setScaleY (iff.read_float ());

			iff.exitChunk ();

		iff.exitForm (TAG_DATA);

	iff.exitForm (TAG_0004);
}

//-------------------------------------------------------------------

void FilterFractal::load_0005 (Iff& iff)
{
	iff.enterForm (TAG_0005);

		//-- load the base data
		LayerItem::load (iff);

		//-- load specific data
		iff.enterForm (TAG_DATA);

			//-- load parameters
			iff.enterChunk (TAG_PARM);

				setFamilyId (iff.read_int32 ());
				setFeatherFunction (static_cast<TerrainGeneratorFeatherFunction> (iff.read_int32 ()));
				setFeatherDistance (clamp (0.f, iff.read_float (), 1.f));
				setLowFractalLimit (iff.read_float ());
				setHighFractalLimit (iff.read_float ());
				setScaleY (iff.read_float ());

			iff.exitChunk ();

		iff.exitForm (TAG_DATA);

	iff.exitForm (TAG_0005);
}

//-------------------------------------------------------------------

void FilterFractal::save (Iff& iff) const
{
	iff.insertForm (TAG_0005);

		//-- save the base
		LayerItem::save (iff);

		//-- save specific data
		iff.insertForm (TAG_DATA);

			iff.insertChunk (TAG (P,A,R,M));

				iff.insertChunkData (getFamilyId ());
				iff.insertChunkData (static_cast<int32> (getFeatherFunction ()));
				iff.insertChunkData (getFeatherDistance ());
				iff.insertChunkData (getLowFractalLimit ());
				iff.insertChunkData (getHighFractalLimit ());
				iff.insertChunkData (getScaleY ());

			iff.exitChunk ();

		iff.exitForm ();

	iff.exitForm ();
}

//-------------------------------------------------------------------

void FilterFractal::setFamilyId (const int newFamilyId)
{
	m_familyId = newFamilyId;
}

//-------------------------------------------------------------------

void FilterFractal::setScaleY (const float newScale)
{
	m_scaleY = newScale;
}

//-------------------------------------------------------------------

void FilterFractal::setLowFractalLimit (const float newLowFractalLimit)
{
	m_lowFractalLimit = newLowFractalLimit;
}

//-------------------------------------------------------------------

void FilterFractal::setHighFractalLimit (const float newHighFractalLimit)
{
	m_highFractalLimit = newHighFractalLimit;
}

//-------------------------------------------------------------------
//
// FilterSlope
//
FilterSlope::FilterSlope () :
	TerrainGenerator::Filter (TAG_FSLP, TGFT_slope),
	maximumAngle (0),
	minimumAngle (0),
	sinMaxAngle (0),
	sinMinAngle (0)
{
	setMinimumAngle (0.f);
	setMaximumAngle (0.f);
}

//-------------------------------------------------------------------

FilterSlope::~FilterSlope ()
{
}

//-------------------------------------------------------------------

float FilterSlope::isWithin (const float /*worldX*/, const float /*worldZ*/, const int x, const int z, const TerrainGenerator::GeneratorChunkData& generatorChunkData) const
{
	const float normalY = generatorChunkData.vertexNormalMap->getData (x, z).y;

	return computeFeatheredInterpolant (sinMaxAngle, normalY, sinMinAngle, getFeatherDistance ());
}

//-------------------------------------------------------------------

void FilterSlope::load (Iff& iff)
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
			DEBUG_FATAL (true, ("unknown layer type %s/%s", buffer, tagBuffer));
		}
		break;
	}
}

//-------------------------------------------------------------------

void FilterSlope::load_0000 (Iff& iff)
{
	iff.enterForm (TAG_0000);

		//-- load the base data
		LayerItem::load (iff);

		//-- load specific data
		iff.enterChunk (TAG_DATA);

			setMinimumAngle (convertDegreesToRadians (iff.read_float ()));
			setMaximumAngle (convertDegreesToRadians (iff.read_float ()));

		iff.exitChunk (TAG_DATA);

	iff.exitForm (TAG_0000);
}

//-------------------------------------------------------------------

void FilterSlope::load_0001 (Iff& iff)
{
	iff.enterForm (TAG_0001);

		//-- load the base data
		LayerItem::load (iff);

		//-- load specific data
		iff.enterChunk (TAG_DATA);

			float unused  = iff.read_float ();
			UNREF (unused);

			setMinimumAngle (convertDegreesToRadians (iff.read_float ()));
			setMaximumAngle (convertDegreesToRadians (iff.read_float ()));

		iff.exitChunk (TAG_DATA);

	iff.exitForm (TAG_0001);
}

//-------------------------------------------------------------------

void FilterSlope::load_0002 (Iff& iff)
{
	iff.enterForm (TAG_0002);

		//-- load the base data
		LayerItem::load (iff);

		//-- load specific data
		iff.enterChunk (TAG_DATA);

			setMinimumAngle (convertDegreesToRadians (iff.read_float ()));
			setMaximumAngle (convertDegreesToRadians (iff.read_float ()));

			setFeatherFunction (static_cast<TerrainGeneratorFeatherFunction> (iff.read_int32 ()));

			const float newFeatherDistance = clamp (0.f, iff.read_float (), 1.f);
			setFeatherDistance (newFeatherDistance);

		iff.exitChunk (TAG_DATA);

	iff.exitForm (TAG_0002);
}

//-------------------------------------------------------------------

void FilterSlope::save (Iff& iff) const
{
	iff.insertForm (TAG_0002);

		//-- save the base
		LayerItem::save (iff);

		//-- save specific data
		iff.insertChunk (TAG_DATA);

			iff.insertChunkData (convertRadiansToDegrees (minimumAngle));
			iff.insertChunkData (convertRadiansToDegrees (maximumAngle));

			iff.insertChunkData (static_cast<int32> (getFeatherFunction ()));
			iff.insertChunkData (getFeatherDistance ());

		iff.exitChunk (TAG_DATA);

	iff.exitForm (TAG_0002);
}

//-------------------------------------------------------------------

void FilterSlope::setMinimumAngle (const float newMinimumAngle)
{
	minimumAngle = clamp (0.f, newMinimumAngle, PI_OVER_2);

	sinMinAngle  = sin (PI_OVER_2 - minimumAngle);
}

//-------------------------------------------------------------------

void FilterSlope::setMaximumAngle (const float newMaximumAngle)
{
	maximumAngle = clamp (0.f, newMaximumAngle, PI_OVER_2);

	sinMaxAngle  = sin (PI_OVER_2 - maximumAngle);
}

//-----------------------------------------------------------------

bool FilterSlope::needsNormals () const
{
	return true;
}

//-------------------------------------------------------------------
//
// FilterDirection
//
FilterDirection::FilterDirection () :
	TerrainGenerator::Filter (TAG_FDIR, TGFT_direction),
	maximumAngle (0),
	minimumAngle (0),
	maximumFeatherAngle (0),
	minimumFeatherAngle (0)
{
}

//-------------------------------------------------------------------

FilterDirection::~FilterDirection ()
{
}

//-------------------------------------------------------------------

float FilterDirection::isWithin (const float /*worldX*/, const float /*worldZ*/, const int x, const int z, const TerrainGenerator::GeneratorChunkData& generatorChunkData) const
{
	const float normalizedTheta = generatorChunkData.vertexNormalMap->getData (x, z).theta () * ooPI_TIMES_2 + 0.5f;

	if (!isWithinRange (minimumFeatherAngle, normalizedTheta, maximumFeatherAngle))
		return 0.f;

	const float featherAngle = (maximumFeatherAngle - minimumFeatherAngle) * getFeatherDistance () * 0.5f;

	if (isWithinRange (minimumFeatherAngle + featherAngle, normalizedTheta, maximumFeatherAngle - featherAngle))
		return 1.f;

	const float maxAngle = maximumFeatherAngle - normalizedTheta;
	const float minAngle = normalizedTheta - minimumFeatherAngle;

	float angle = featherAngle;

	if (maxAngle < angle)
		angle = maxAngle;

	if (minAngle < angle)
		angle = minAngle;

	return angle / featherAngle;
}

//-------------------------------------------------------------------

void FilterDirection::load (Iff& iff)
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
			DEBUG_FATAL (true, ("unknown layer type %s/%s", buffer, tagBuffer));
		}
		break;
	}
}

//-------------------------------------------------------------------

void FilterDirection::load_0000 (Iff& iff)
{
	iff.enterForm (TAG_0000);

		//-- load the base data
		LayerItem::load (iff);

		//-- load specific data
		iff.enterChunk (TAG_DATA);

			setMinimumAngle (convertDegreesToRadians (iff.read_float ()));
			setMaximumAngle (convertDegreesToRadians (iff.read_float ()));

			setFeatherFunction (static_cast<TerrainGeneratorFeatherFunction> (iff.read_int32 ()));

			const float newFeatherDistance = clamp (0.f, iff.read_float (), 1.f);
			setFeatherDistance (newFeatherDistance);

		iff.exitChunk (TAG_DATA);

	iff.exitForm (TAG_0000);
}

//-------------------------------------------------------------------

void FilterDirection::save (Iff& iff) const
{
	iff.insertForm (TAG_0000);

		//-- save the base
		LayerItem::save (iff);

		//-- save specific data
		iff.insertChunk (TAG_DATA);

			iff.insertChunkData (convertRadiansToDegrees (minimumAngle));
			iff.insertChunkData (convertRadiansToDegrees (maximumAngle));

			iff.insertChunkData (static_cast<int32> (getFeatherFunction ()));
			iff.insertChunkData (getFeatherDistance ());

		iff.exitChunk (TAG_DATA);

	iff.exitForm (TAG_0000);
}

//-------------------------------------------------------------------

void FilterDirection::setMinimumAngle (float newMinimumAngle)
{
	minimumAngle = std::max (-PI, newMinimumAngle);

	minimumFeatherAngle = minimumAngle * ooPI_TIMES_2;
}

//-------------------------------------------------------------------

void FilterDirection::setMaximumAngle (float newMaximumAngle)
{
	maximumAngle = std::min (PI, newMaximumAngle);

	maximumFeatherAngle = maximumAngle * ooPI_TIMES_2;
}

//-----------------------------------------------------------------

bool FilterDirection::needsNormals () const
{
	return true;
}

//-------------------------------------------------------------------
//
// FilterShader
//
FilterShader::FilterShader () :	
	TerrainGenerator::Filter (TAG_FSHD, TGFT_shader),
	familyId (0)
{
}

//-------------------------------------------------------------------

FilterShader::~FilterShader ()
{
}

//-------------------------------------------------------------------

void FilterShader::setFamilyId (const int newFamilyId)
{
	familyId = newFamilyId;
}

//-------------------------------------------------------------------

bool FilterShader::needsShaders () const
{
	return true;
}

//-------------------------------------------------------------------

float FilterShader::isWithin (const float /*worldX*/, const float /*worldZ*/, const int x, const int z, const TerrainGenerator::GeneratorChunkData& generatorChunkData) const
{
	ShaderGroup::Info sgi = generatorChunkData.shaderMap->getData (x, z);

	return (sgi.getFamilyId () == familyId) ? 1.f : 0.f;
}

//-------------------------------------------------------------------

void FilterShader::load (Iff& iff)
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
			DEBUG_FATAL (true, ("unknown layer type %s/%s", buffer, tagBuffer));
		}
		break;
	}
}
//-------------------------------------------------------------------

void FilterShader::save (Iff& iff) const
{
	iff.insertForm (TAG_0000);

		//-- save the base
		LayerItem::save (iff);

		//-- save specific data
		iff.insertChunk (TAG_DATA);

			iff.insertChunkData (familyId);

		iff.exitChunk (TAG_DATA);

	iff.exitForm (TAG_0000);
}

//-------------------------------------------------------------------

void FilterShader::load_0000 (Iff& iff)
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

//-------------------------------------------------------------------
//
// FilterBitmap
//

FilterBitmap::FilterBitmap () :
	TerrainGenerator::Filter (TAG_FBIT, TGFT_bitmap),
	m_familyId (0),
	m_lowBitmapLimit (0),
	m_highBitmapLimit (0),
	m_extent(),
	m_gain (0)
{
}

//-------------------------------------------------------------------

FilterBitmap::~FilterBitmap ()
{
	
}

float FilterBitmap::isWithin (const float worldX, const float worldZ, const int /*x*/, const int /*z*/, const TerrainGenerator::GeneratorChunkData& generatorChunkData) const
{
	const Image* image = generatorChunkData.bitmapGroup->getFamilyBitmap(m_familyId);

	if(!image)
	{
		return 1.0f;
	}

	const int imageWidth = image->getWidth();
	const int imageHeight = image->getHeight();
	const int imageStride = image->getStride();
	Rectangle2d rect;
	rect.x0 = 0.0f;
	rect.y0 = 0.0f;
	rect.x1 = m_extent.x1 - m_extent.x0;
	rect.y1 = m_extent.y1 - m_extent.y0;
	
	DEBUG_FATAL((rect.x1 == 0.0f),("FilterBitmap::isWithin: rect.x1 is 0.0f"));
	DEBUG_FATAL((rect.y1 == 0.0f),("FilterBitmap::isWithin: rect.y1 is 0.0f"));

	const float scaledWorldX = std::min((worldX - m_extent.x0) * imageWidth/rect.x1,(float)imageWidth - 1.0f);
	const float scaledWorldY = std::min((worldZ - m_extent.y0) * imageHeight/rect.y1, (float)imageHeight - 1.0f);

	const int x0 = (int)scaledWorldX;
	const int y0 = (int)scaledWorldY;
	int x1 = std::min(x0 + 1,imageWidth - 1);
	int y1 = std::min(y0 + 1,imageHeight - 1);

	const int indexIntoImage00 = (((imageHeight - 1) - y0) * imageStride) + x0*(imageStride/imageWidth);
	const int indexIntoImage10 = (((imageHeight - 1) - y0) * imageStride) + x1*(imageStride/imageWidth);
	const int indexIntoImage01 = (((imageHeight - 1) - y1) * imageStride) + x0*(imageStride/imageWidth);
	const int indexIntoImage11 = (((imageHeight - 1) - y1) * imageStride) + x1*(imageStride/imageWidth);
	
	const uint8* data = image->lockReadOnly ();

	const uint8 value00 = data[indexIntoImage00];
	const uint8 value10 = data[indexIntoImage10];
	const uint8 value01 = data[indexIntoImage01];
	const uint8 value11 = data[indexIntoImage11];

	image->unlock();

	const float normalizeVal = 1.0f/255.0f;
	const float bitmapHeight00 = value00 * normalizeVal;
	const float bitmapHeight10 = value10 * normalizeVal;
	const float bitmapHeight01 = value01 * normalizeVal;
	const float bitmapHeight11 = value11 * normalizeVal;

	float dx1 = scaledWorldX - (int)scaledWorldX;
	float dx2 = 1.0f - dx1;
	float dy1 = scaledWorldY - (int)scaledWorldY;
	float dy2 = 1.0f - dy1;

	float bitmapHeight = bitmapHeight00 * (dx2 * dy2)
		+ bitmapHeight10 * (dx1 * dy2)
		+ bitmapHeight01 * (dx2 * dy1)
		+ bitmapHeight11 * (dx1 * dy1);

	
	bitmapHeight += m_gain;
	if(bitmapHeight < 0.0f)
	{
		bitmapHeight = 0.0f;
	}
	else if(bitmapHeight >= 1.0f)
	{
		bitmapHeight = 0.99999f; // to get around the upper exclusion check in computeFeatheredInterpolant
	}

	return computeFeatheredInterpolant (m_lowBitmapLimit, bitmapHeight, m_highBitmapLimit, getFeatherDistance ()) * bitmapHeight;


}

//-------------------------------------------------------------------

void FilterBitmap::setExtent(const Rectangle2d& rect)
{
	m_extent = rect;
}

//-------------------------------------------------------------------

void FilterBitmap::setFamilyId (const int newFamilyId)
{
	m_familyId = newFamilyId;
}

//-------------------------------------------------------------------

void FilterBitmap::setLowBitmapLimit (const float newLowBitmapLimit)
{
	m_lowBitmapLimit = newLowBitmapLimit;
}

//-------------------------------------------------------------------

void FilterBitmap::setHighBitmapLimit (const float newHighBitmapLimit)
{
	m_highBitmapLimit = newHighBitmapLimit;
}

//-------------------------------------------------------------------

void FilterBitmap::setGain (const float newValue)
{
	m_gain = newValue;
}

//-------------------------------------------------------------------

void FilterBitmap::load (Iff& iff/*, BitmapGroup& bitmapGroup*/)
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
			DEBUG_FATAL (true, ("unknown layer type %s/%s", buffer, tagBuffer));
		}
		break;
	}
}

void FilterBitmap::load_0000 (Iff& iff)
{
	iff.enterForm (TAG_0000);

		//-- load the base data
		LayerItem::load (iff);

		//-- load specific data
		iff.enterForm (TAG_DATA);

			//-- load parameters
			iff.enterChunk (TAG_PARM);

				setFamilyId (iff.read_int32 ());
				setFeatherFunction (static_cast<TerrainGeneratorFeatherFunction> (iff.read_int32 ()));
				setFeatherDistance (clamp (0.f, iff.read_float (), 1.f));
				setLowBitmapLimit (iff.read_float ());
				setHighBitmapLimit (iff.read_float ());

			iff.exitChunk ();

		iff.exitForm (TAG_DATA);

	iff.exitForm (TAG_0000);
}

void FilterBitmap::load_0001 (Iff& iff)
{
	iff.enterForm (TAG_0001);

		//-- load the base data
		LayerItem::load (iff);

		//-- load specific data
		iff.enterForm (TAG_DATA);

			//-- load parameters
			iff.enterChunk (TAG_PARM);

				setFamilyId (iff.read_int32 ());
				setFeatherFunction (static_cast<TerrainGeneratorFeatherFunction> (iff.read_int32 ()));
				setFeatherDistance (clamp (0.f, iff.read_float (), 1.f));
				setLowBitmapLimit (iff.read_float ());
				setHighBitmapLimit (iff.read_float ());
				setGain (iff.read_float());

			iff.exitChunk ();

		iff.exitForm (TAG_DATA);

	iff.exitForm (TAG_0001);
}



void FilterBitmap::save (Iff& iff) const
{
	iff.insertForm (TAG_0001);

		//-- save the base
		LayerItem::save (iff);

		//-- save specific data
		iff.insertForm (TAG_DATA);

			iff.insertChunk (TAG (P,A,R,M));

				iff.insertChunkData (getFamilyId ());
				iff.insertChunkData (static_cast<int32> (getFeatherFunction ()));
				iff.insertChunkData (getFeatherDistance ());
				iff.insertChunkData (getLowBitmapLimit ());
				iff.insertChunkData (getHighBitmapLimit ());
				iff.insertChunkData (getGain ());

			iff.exitChunk ();

		iff.exitForm ();

	iff.exitForm ();
}

