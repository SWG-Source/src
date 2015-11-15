// ======================================================================
//
// CompressedQuaternion.cpp
// Copyright 2002 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#include "sharedMath/FirstSharedMath.h"
#include "sharedMath/CompressedQuaternion.h"

#include "sharedMath/Quaternion.h"

#include <algorithm>
#include <limits>
#include <vector>

// ======================================================================

#if DEBUG_LEVEL == DEBUG_LEVEL_DEBUG

	#define VERIFY_COMPRESSION  1

#else

	#define VERIFY_COMPRESSION  0

#endif

// ======================================================================

namespace CompressedQuaternionNamespace
{
	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	struct FormatPrecisionInfo
	{
		//-- Specified directly.
		uint8  formatId;
		uint8  baseIndexMask;
		int    baseCount;
		float  baseSeparation;

		//-- Calculated.
		float  compressFactorElevenBit;
		float  expandFactorElevenBit;

		float  compressFactorTenBit;
		float  expandFactorTenBit;
	};

	#define MAKE_BASE_SEPARATION(baseShiftCount) (2.0f / static_cast<float>((0x01 << (baseShiftCount)) + 1))

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	class FormatData
	{
	public:

		FormatData();

		void    install(float baseValue, uint8 formatPrecisionIndex);

		uint32  compressTenBit(float uncompressedValue) const;
		uint32  compressElevenBit(float uncompressedValue) const;

		float   expandTenBit(uint32 compressedValue) const;
		float   expandElevenBit(uint32 compressedValue) const;

	private:
	
		float  m_baseValue;
		uint8  m_formatPrecisionIndex;

#ifdef _DEBUG
		bool   m_installed;
#endif

	};

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	// packed format: [MSB] x-11-bit  y-11-bit  z-10-bit
	const uint32 cs_xShift = 21;
	const uint32 cs_yShift = 10;

	// Acceptable error in given w calculation from real w calculation.
	const float cs_xAcceptableEpsilon  = 0.001f;
	const float cs_yAcceptableEpsilon  = 0.001f;
	const float cs_zAcceptableEpsilon  = 2.0f * cs_xAcceptableEpsilon;
	const float cs_wAcceptableEpsilon  = 0.1f;

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	// 11-bit compressed format
	const uint32 cs_valueMaskElevenBit     = BINARY3(0011, 1111, 1111);
	const uint32 cs_signBitElevenBit       = BINARY3(0100, 0000, 0000);

	// 10-bit compressed format
	const uint32 cs_valueMaskTenBit        = BINARY3(0001, 1111, 1111);
	const uint32 cs_signBitTenBit          = BINARY3(0010, 0000, 0000);


	const int    cs_minFormatValue         = 0;
	const int    cs_maxFormatValue         = 254;

	#define MAKE_PRECISION_INFO(formatId, baseIndexMask, baseShiftCount)  {formatId, baseIndexMask, 0x01 << baseShiftCount, MAKE_BASE_SEPARATION(baseShiftCount), 0.0f, 0.0f, 0.0f, 0.0f}

	FormatPrecisionInfo  s_formatPrecisionInfo[] =
		{
			MAKE_PRECISION_INFO(BINARY2(1111, 1110), BINARY2(0000, 0000), 0),
			MAKE_PRECISION_INFO(BINARY2(1111, 1100), BINARY2(0000, 0001), 1),
			MAKE_PRECISION_INFO(BINARY2(1111, 1000), BINARY2(0000, 0011), 2),
			MAKE_PRECISION_INFO(BINARY2(1111, 0000), BINARY2(0000, 0111), 3),
			MAKE_PRECISION_INFO(BINARY2(1110, 0000), BINARY2(0000, 1111), 4),
			MAKE_PRECISION_INFO(BINARY2(1100, 0000), BINARY2(0001, 1111), 5),
			MAKE_PRECISION_INFO(BINARY2(1000, 0000), BINARY2(0011, 1111), 6)
		};

	const int    cs_minBaseShiftCount      = 0;
	const int    cs_maxBaseShiftCount      = static_cast<int>(sizeof(s_formatPrecisionInfo) / sizeof(s_formatPrecisionInfo[0])) - 1;

	FormatData   s_formatData[cs_maxFormatValue + 1];

	bool         s_installed;

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	int    convertShiftToCount(int shift);
	float  calculateRange(int baseShiftCount);

	void   findClosestBase(int baseShiftCount, float midpoint, int &baseIndex, float &baseValue);
	int    findBaseShiftCountCoveringRange(float range);
	bool   findFormatForRange(int baseShiftCount, float minValue, float maxValue, uint8 &format);

	uint32 doCompress(float w, float x, float y, float z, uint8 xFormat, uint8 yFormat, uint8 zFormat);
	void   doExpand(uint32 data, uint8 xFormat, uint8 yFormat, uint8 zFormat, float &w, float &x, float &y, float &z);
}

using namespace CompressedQuaternionNamespace;

// ======================================================================
// class CompressedQuaternionNamespace::FormatData
// ======================================================================

CompressedQuaternionNamespace::FormatData::FormatData() :
	m_baseValue(0),
	m_formatPrecisionIndex(0)
#ifdef _DEBUG
	, m_installed(false)
#endif
{
}

// ----------------------------------------------------------------------

void CompressedQuaternionNamespace::FormatData::install(float baseValue, uint8 formatPrecisionIndex)
{
	VALIDATE_RANGE_INCLUSIVE_INCLUSIVE(cs_minBaseShiftCount, static_cast<int>(formatPrecisionIndex), cs_maxBaseShiftCount);

	m_baseValue            = baseValue;
	m_formatPrecisionIndex = formatPrecisionIndex;

#ifdef _DEBUG
	m_installed            = true;
#endif
}

// ----------------------------------------------------------------------

uint32 CompressedQuaternionNamespace::FormatData::compressTenBit(float uncompressedValue) const
{
	DEBUG_FATAL(!m_installed, ("format not installed."));

	if (uncompressedValue >= m_baseValue)
	{
		const uint32 rawValue = static_cast<uint32>(s_formatPrecisionInfo[m_formatPrecisionIndex].compressFactorTenBit * std::max(0.0f, uncompressedValue - m_baseValue));
		return std::min(cs_valueMaskTenBit, rawValue);
	}
	else
	{
		const uint32 rawValue = static_cast<uint32>(s_formatPrecisionInfo[m_formatPrecisionIndex].compressFactorTenBit * std::max(0.0f, m_baseValue - uncompressedValue));
		return cs_signBitTenBit | std::min(cs_valueMaskTenBit, rawValue);
	}
}

// ----------------------------------------------------------------------

uint32 CompressedQuaternionNamespace::FormatData::compressElevenBit(float uncompressedValue) const
{
	DEBUG_FATAL(!m_installed, ("format not installed."));

	if (uncompressedValue >= m_baseValue)
	{
		const uint32 rawValue = static_cast<uint32>(s_formatPrecisionInfo[m_formatPrecisionIndex].compressFactorElevenBit * std::max(0.0f, uncompressedValue - m_baseValue));
		return std::min(cs_valueMaskElevenBit, rawValue);
	}
	else
	{
		const uint32 rawValue = static_cast<uint32>(s_formatPrecisionInfo[m_formatPrecisionIndex].compressFactorElevenBit * std::max(0.0f, m_baseValue - uncompressedValue));
		return cs_signBitElevenBit | std::min(cs_valueMaskElevenBit, rawValue);
	}
}

// ----------------------------------------------------------------------
// This function works properly with any kind of junk outside the lowest 10 bits.  You do not need to mask the parameter prior to calling.

float CompressedQuaternionNamespace::FormatData::expandTenBit(uint32 compressedValue) const
{
	DEBUG_FATAL(!m_installed, ("format not installed."));

	if ((compressedValue & cs_signBitTenBit) != 0)
		return m_baseValue - (static_cast<float>(compressedValue & cs_valueMaskTenBit) * s_formatPrecisionInfo[m_formatPrecisionIndex].expandFactorTenBit);
	else
		return m_baseValue + (static_cast<float>(compressedValue & cs_valueMaskTenBit) * s_formatPrecisionInfo[m_formatPrecisionIndex].expandFactorTenBit);
}

// ----------------------------------------------------------------------
// This function works properly with any kind of junk outside the lowest 11 bits.  You do not need to mask the parameter prior to calling.

float CompressedQuaternionNamespace::FormatData::expandElevenBit(uint32 compressedValue) const
{
	DEBUG_FATAL(!m_installed, ("format not installed."));

	if ((compressedValue & cs_signBitElevenBit) != 0)
		return m_baseValue - (static_cast<float>(compressedValue & cs_valueMaskElevenBit) * s_formatPrecisionInfo[m_formatPrecisionIndex].expandFactorElevenBit);
	else
		return m_baseValue + (static_cast<float>(compressedValue & cs_valueMaskElevenBit) * s_formatPrecisionInfo[m_formatPrecisionIndex].expandFactorElevenBit);
}

// ======================================================================

inline int CompressedQuaternionNamespace::convertShiftToCount(int shift)
{
	VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE(0, shift, 31);
	return (0x01 << static_cast<uint8>(shift));
}

// ----------------------------------------------------------------------

inline float CompressedQuaternionNamespace::calculateRange(int baseShiftCount)
{
	DEBUG_FATAL(baseShiftCount < 0, ("bad baseShiftCount arg [%d].", baseShiftCount));
	return 4.0f / static_cast<float>(convertShiftToCount(baseShiftCount) + 1);
}

// ----------------------------------------------------------------------

void CompressedQuaternionNamespace::findClosestBase(int baseShiftCount, float midpoint, int &baseIndex, float &baseValue)
{
	VALIDATE_RANGE_INCLUSIVE_INCLUSIVE(cs_minBaseShiftCount, baseShiftCount, cs_maxBaseShiftCount);

	//-- Brute force, this could be far more intelligent.
	const int   baseCount      = s_formatPrecisionInfo[baseShiftCount].baseCount;
	const float baseSeparation = s_formatPrecisionInfo[baseShiftCount].baseSeparation;

	float     closestBaseDistance = std::numeric_limits<float>::max();
	float     closestBaseValue    = std::numeric_limits<float>::max();
	int       closestBaseIndex    = -1;

	for (int testBaseIndex = 0; testBaseIndex < baseCount; ++testBaseIndex)
	{
		const float testBaseValue = -1.0f + (testBaseIndex + 1) * baseSeparation;
		const float testDistance  = std::abs(testBaseValue - midpoint);

		if (testDistance < closestBaseDistance)
		{
			closestBaseDistance = testDistance;
			closestBaseValue    = testBaseValue;
			closestBaseIndex    = testBaseIndex;
		}
		else
		{
			//-- We're getting farther away, stop now.
			break;
		}
	}

	baseIndex = closestBaseIndex;
	baseValue = closestBaseValue;
}

// ----------------------------------------------------------------------

int CompressedQuaternionNamespace::findBaseShiftCountCoveringRange(float range)
{
	for (int baseShiftCount = cs_maxBaseShiftCount; baseShiftCount >= 0; --baseShiftCount)
	{
		const float baseCountRange = calculateRange(baseShiftCount);
		if (baseCountRange >= range)
		{
			//-- We found the tightest-fitting base count that is at least large enough to handle the specified range.
			//   We do this so that we have the greatest precision available over that tightest-fitting range.
			return baseShiftCount;
		}
	}

	DEBUG_FATAL(true, ("Failed to find a base count that handles the range [%g].", range));
	return -1; //lint !e527 // unreachable // reachable in release.
}

// ----------------------------------------------------------------------

bool CompressedQuaternionNamespace::findFormatForRange(int baseShiftCount, float minValue, float maxValue, uint8 &format)
{
	VALIDATE_RANGE_INCLUSIVE_INCLUSIVE(cs_minBaseShiftCount, baseShiftCount, cs_maxBaseShiftCount);
	DEBUG_FATAL(minValue > maxValue, ("minValue [%g] > maxValue [%g].", minValue, maxValue));

	//-- Find the user range and midpoint.
	const float range    = maxValue - minValue;
	const float midpoint = minValue + 0.5f * range;

	//-- Find this format's closest base to the midpoint.
	int    baseIndex = -1;
	float  baseValue = 0.0f;

	findClosestBase(baseShiftCount, midpoint, baseIndex, baseValue);

	//-- Check if the user range fits within this format's base and range.
	const float formatHalfRange = 0.5f * calculateRange(baseShiftCount);
	const bool userRangeFitFormat = ((minValue >= (baseValue - formatHalfRange)) && (maxValue <= (baseValue + formatHalfRange)));

	if (!userRangeFitFormat)
		return false;

	//-- Compute the format value from this information.
	DEBUG_FATAL(static_cast<int>(s_formatPrecisionInfo[baseShiftCount].baseIndexMask & static_cast<uint8>(baseIndex)) != baseIndex, ("base index %d not valid for format with baseShift = %d.", baseIndex, baseShiftCount));
	format = static_cast<uint8>(s_formatPrecisionInfo[baseShiftCount].formatId | static_cast<uint8>(baseIndex));

	return true;
}

// ----------------------------------------------------------------------

uint32 CompressedQuaternionNamespace::doCompress(float w, float x, float y, float z, uint8 xFormat, uint8 yFormat, uint8 zFormat)
{
	//-- Flip the quaternion if w is negative so we don't need to store a sign bit for w.
	if (w < 0.0f)
	{
		w = -w;
		x = -x;
		y = -y;
		z = -z;
	}

	//-- Ensure we are compressing a unit quaternion.
	VALIDATE_RANGE_INCLUSIVE_INCLUSIVE(-1.0f, x, 1.0f);
	VALIDATE_RANGE_INCLUSIVE_INCLUSIVE(-1.0f, y, 1.0f);
	VALIDATE_RANGE_INCLUSIVE_INCLUSIVE(-1.0f, z, 1.0f);

#ifdef _DEBUG
	// If w is small enough, we won't be able to take the square root.
	if (std::abs(w) >= cs_wAcceptableEpsilon)
	{
		const float calculatedW = sqrt(1.0f - (x*x + y*y + z*z));
		DEBUG_FATAL(!WithinEpsilonInclusive(calculatedW, w, cs_wAcceptableEpsilon), ("Quaternion (w=%g,x=%g,y=%g,z=%g) does not appear to be a unit quaternion.", w, x, y, z));
	}
#endif

	//-- Pack the values.
	const uint32 xPacked = s_formatData[xFormat].compressElevenBit(x);
	const uint32 yPacked = s_formatData[yFormat].compressElevenBit(y);
	const uint32 zPacked = s_formatData[zFormat].compressTenBit(z);

	//-- Shift and combine.
	return (xPacked << cs_xShift) | (yPacked << cs_yShift) | zPacked;
}

// ----------------------------------------------------------------------

void CompressedQuaternionNamespace::doExpand(uint32 data, uint8 xFormat, uint8 yFormat, uint8 zFormat, float &w, float &x, float &y, float &z)
{
	//-- Expand the components.
	x = s_formatData[xFormat].expandElevenBit(data >> cs_xShift);
	y = s_formatData[yFormat].expandElevenBit(data >> cs_yShift);
	z = s_formatData[zFormat].expandTenBit(data);

	//-- Calculate w.
	// @todo consider a faster square root approximation function.
	w = sqrt(1.0f - (x*x + y*y + z*z));
}

// ======================================================================
// class CompressedQuaternion: static public member functions
// ======================================================================

void CompressedQuaternion::install()
{
	//-- Calculate the data for quaternion (de)compression.
	
	for (int baseShiftCount = 0; baseShiftCount <= cs_maxBaseShiftCount; ++baseShiftCount)
	{
		float const baseSeparation = s_formatPrecisionInfo[baseShiftCount].baseSeparation;
		float const halfRange      = 0.5f * calculateRange(baseShiftCount);
		DEBUG_FATAL(halfRange <= 0.0f, ("bad half range [%g].", halfRange));

		// compression factor is : uncompressedUnits * (total compressedUnits/ total uncompressedUnits) = compressedUnits
		s_formatPrecisionInfo[baseShiftCount].compressFactorElevenBit = static_cast<float>(BINARY3(0011, 1111, 1111)) / halfRange;
		s_formatPrecisionInfo[baseShiftCount].expandFactorElevenBit   = halfRange / static_cast<float>(BINARY3(0011, 1111, 1111));

		s_formatPrecisionInfo[baseShiftCount].compressFactorTenBit    = static_cast<float>(BINARY3(0001, 1111, 1111)) / halfRange;
		s_formatPrecisionInfo[baseShiftCount].expandFactorTenBit      = halfRange / static_cast<float>(BINARY3(0001, 1111, 1111));

		uint8 const formatId = s_formatPrecisionInfo[baseShiftCount].formatId;

		int const baseCount = s_formatPrecisionInfo[baseShiftCount].baseCount;
		VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE(0, baseCount, cs_maxFormatValue);

		for (int i = 0; i < baseCount; ++i)
		{
			uint8 const formatIndex = static_cast<uint8>(formatId | static_cast<uint8>(i));
			float const baseValue   = - 1.0f + (i + 1) * baseSeparation;

			VALIDATE_RANGE_INCLUSIVE_INCLUSIVE(0, static_cast<int>(formatIndex), cs_maxFormatValue);
			s_formatData[formatIndex].install(baseValue, static_cast<uint8>(baseShiftCount));
		}
	}

	s_installed = true;
}

// ----------------------------------------------------------------------

/**
 * Determine the most precise compression format for a quaternion component
 * that will cover the specified range of value.
 *
 * Our quaternion compression mechanism uses a fixed number of output bits 
 * to represent a component of a quaternion.  We let the level of precision
 * vary based on the range of values that the quaternion component needs to
 * represent over time.  
 *
 * The caller should do something like this.  For each quaternion component
 * that is to be compressed, find the range of values that the component takes
 * on across the quaternions that will be compressed with the same compression
 * format.  Feed the min and max value into this function, then store the
 * returned format to be used during compression and decompression for that
 * particular component.  This needs to be done for the x, y and z components,
 * but not the w component.  We calculate the w component from the x, y and z.
 * We can do this because we are using unit quaternions.
 */

uint8 CompressedQuaternion::getOptimalCompressionFormat(float minValue, float maxValue)
{
	DEBUG_FATAL(minValue > maxValue, ("min and max are not set properly."));

	//-- Find the largest division count (= highest precision compressed representation)
	//   that can represent values over the specified range.
	int   baseShiftCount = findBaseShiftCountCoveringRange(maxValue - minValue);
	uint8 format          = 255;

	for (; (baseShiftCount > -1) && !findFormatForRange(baseShiftCount, minValue, maxValue, format); --baseShiftCount)
	{
	}

	DEBUG_FATAL(baseShiftCount < 0, ("failed to find an encoding for range [%g, %g].", minValue, maxValue));
	VALIDATE_RANGE_INCLUSIVE_INCLUSIVE(cs_minFormatValue, static_cast<int>(format), cs_maxFormatValue);

	return format;
}

// ----------------------------------------------------------------------
/**
 * Find optimal compression format for each x, y and z component of the
 * specified source rotations.
 */

void CompressedQuaternion::getOptimalCompressionFormat(const QuaternionVector &sourceRotations, uint8 &xFormat, uint8 &yFormat, uint8 &zFormat)
{
	//-- Handle no source rotations.
	if (sourceRotations.empty())
	{
		DEBUG_WARNING(true, ("getOptimalCompressionFormat(): sourceRotations container is empty, returning least precise format."));

		// Return least precise format because that is the only thing guaranteed to cover na
		xFormat = s_formatPrecisionInfo[0].formatId;
		yFormat = s_formatPrecisionInfo[0].formatId;
		zFormat = s_formatPrecisionInfo[0].formatId;
		return;
	}

	//-- Collect min and max component values for the rotations.
	float minX = std::numeric_limits<float>::max();
	float maxX = -std::numeric_limits<float>::max();

	float minY = std::numeric_limits<float>::max();
	float maxY = -std::numeric_limits<float>::max();

	float minZ = std::numeric_limits<float>::max();
	float maxZ = -std::numeric_limits<float>::max();

	const QuaternionVector::const_iterator endIt = sourceRotations.end();
	for (QuaternionVector::const_iterator it = sourceRotations.begin(); it != endIt; ++it)
	{
		// Get the quaternion.
		Quaternion rotation = *it;

		// Flip quaternion if w < 0.
		if (rotation.w < 0.0f)
		{
			rotation.x = -rotation.x;
			rotation.y = -rotation.y;
			rotation.z = -rotation.z;
		}

		// Update the min and max component values.
		minX = std::min(minX, rotation.x);
		maxX = std::max(maxX, rotation.x);

		minY = std::min(minY, rotation.y);
		maxY = std::max(maxY, rotation.y);

		minZ = std::min(minZ, rotation.z);
		maxZ = std::max(maxZ, rotation.z);
	}

	xFormat = getOptimalCompressionFormat(minX, maxX);
	yFormat = getOptimalCompressionFormat(minY, maxY);
	zFormat = getOptimalCompressionFormat(minZ, maxZ);
}

// ----------------------------------------------------------------------

void CompressedQuaternion::compressRotations(const QuaternionVector &sourceRotations, uint8 xFormat, uint8 yFormat, uint8 zFormat, CompressedQuaternionVector &compressedRotations)
{
	//-- Adjust destination vector size.
	compressedRotations.clear();
	compressedRotations.reserve(sourceRotations.size());

	//-- Convert each source rotation to a destination rotation.
	const QuaternionVector::const_iterator endIt = sourceRotations.end();
	for (QuaternionVector::const_iterator it = sourceRotations.begin(); it != endIt; ++it)
	{
		compressedRotations.push_back(CompressedQuaternion(*it, xFormat, yFormat, zFormat));

#if VERIFY_COMPRESSION

		const CompressedQuaternion &compressedRotation = compressedRotations.back();
		const Quaternion            expandedRotation   = compressedRotation.expand(xFormat, yFormat, zFormat);

		Quaternion sourceRotation     = *it;
		if (sourceRotation.w < 0.0f)
		{
			sourceRotation.w = -sourceRotation.w;
			sourceRotation.x = -sourceRotation.x;
			sourceRotation.y = -sourceRotation.y;
			sourceRotation.z = -sourceRotation.z;
		}

		const float deltaW = std::abs(expandedRotation.w - sourceRotation.w);
		const float deltaX = std::abs(expandedRotation.x - sourceRotation.x);
		const float deltaY = std::abs(expandedRotation.y - sourceRotation.y);
		const float deltaZ = std::abs(expandedRotation.z - sourceRotation.z);

		if ( (deltaW > cs_wAcceptableEpsilon) || 
			(deltaX > cs_xAcceptableEpsilon) ||
			(deltaY > cs_yAcceptableEpsilon) ||
			(deltaZ > cs_zAcceptableEpsilon))
		{
			//-- Let's do it again.  Make it easier to debug.
			const CompressedQuaternion  cq2(sourceRotation, xFormat, yFormat, zFormat);
			const Quaternion            eq2 = cq2.expand(xFormat, yFormat, zFormat);

			UNREF(cq2);
			UNREF(eq2);

			DEBUG_FATAL(true, ("compression data distortion. [source=(%g,%g,%g,%g),dest=(%g,%g,%g,%g)].",
				sourceRotation.w, sourceRotation.x, sourceRotation.y, sourceRotation.z,
				expandedRotation.w, expandedRotation.x, expandedRotation.y, expandedRotation.z));
		}

#endif
	}
}

// ======================================================================

CompressedQuaternion::CompressedQuaternion(uint32 compressedValue) :
	m_data(compressedValue)
{
}

// ----------------------------------------------------------------------

CompressedQuaternion::CompressedQuaternion(const Quaternion &rhs, uint8 xFormat, uint8 yFormat, uint8 zFormat) :
	m_data(doCompress(rhs.w, rhs.x, rhs.y, rhs.z, xFormat, yFormat, zFormat))
{
}

// ----------------------------------------------------------------------

CompressedQuaternion::CompressedQuaternion(float w, float x, float y, float z, uint8 xFormat, uint8 yFormat, uint8 zFormat) :
	m_data(doCompress(w, x, y, z, xFormat, yFormat, zFormat))
{
}

// ----------------------------------------------------------------------

Quaternion CompressedQuaternion::expand(uint8 xFormat, uint8 yFormat, uint8 zFormat) const
{
	float w;
	float x;
	float y;
	float z;

	doExpand(m_data, xFormat, yFormat, zFormat, w, x, y, z);
	return Quaternion(w, x, y, z);
}

// ----------------------------------------------------------------------

void CompressedQuaternion::expand(uint8 xFormat, uint8 yFormat, uint8 zFormat, Quaternion &destination) const
{
	doExpand(m_data, xFormat, yFormat, zFormat, destination.w, destination.x, destination.y, destination.z);
}

// ----------------------------------------------------------------------

void CompressedQuaternion::expand(uint8 xFormat, uint8 yFormat, uint8 zFormat, float &w, float &x, float &y, float &z) const
{
	doExpand(m_data, xFormat, yFormat, zFormat, w, x, y, z);
}

// ----------------------------------------------------------------------

uint32 CompressedQuaternion::getCompressedValue() const
{
	return m_data;
}

// ----------------------------------------------------------------------

void CompressedQuaternion::debugDump() const
{
	DEBUG_REPORT_LOG(true, ("[data=0x%08x]\n", static_cast<unsigned int>(m_data)));
}

// ======================================================================
