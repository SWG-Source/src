// ======================================================================
//
// RangedIntCustomizationVariable.cpp
// Copyright 2002 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#include "sharedObject/FirstSharedObject.h"
#include "sharedObject/RangedIntCustomizationVariable.h"
#include "sharedObject/CustomizationData.h"

#include <algorithm>
#include <limits>
#include <string>
#include <vector>
#include <cstdio>

// ======================================================================

bool RangedIntCustomizationVariable::doesVariablePersist() const
{
	// all ranged ints are meant to persist
	return true;
}

// ----------------------------------------------------------------------

int RangedIntCustomizationVariable::getPersistedDataByteCount() const
{
	//-- Get the range.
	int minInclusive = 0;
	int maxExclusive = 0;

	getRange(minInclusive, maxExclusive);

	if ((minInclusive >= 0) && (minInclusive < 255) && (maxExclusive > 0) && (maxExclusive <= 256))
	{
		// Value fits in a single unsigned byte.
		return 1;
	}
	else
	{
		//-- If either of these get hit, we've got to upgrade the customization data format.
		VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE(std::numeric_limits<int16>::min(), static_cast<int16>(minInclusive), std::numeric_limits<int16>::max());
		VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE(static_cast<int>(std::numeric_limits<int16>::min()), maxExclusive, static_cast<int>(std::numeric_limits<int16>::max()) + 1);

		// Value fits in a signed 16-bit int.
		return 2;
	}
}

// ----------------------------------------------------------------------

void RangedIntCustomizationVariable::saveToByteVector(ByteVector &data) const
{
	int const byteCount = getPersistedDataByteCount();
	switch (byteCount)
	{
		case 1:
		{
			// Store as single unsigned byte.
			byte const value = static_cast<byte>(getValue());
			data.push_back(value);
			break;
		}

		case 2:
		{
			// Treated as little-endian signed 16-bit number (least significant byte earliest/first in memory).
			int16 const value = static_cast<int16>(getValue());
			
			byte const lowerByte = static_cast<byte>(static_cast<uint16>(value) & 0x00ff);
			data.push_back(lowerByte);
			
			byte const upperByte = static_cast<byte>((static_cast<uint16>(value) >> 8) & 0x00ff);
			data.push_back(upperByte);
			break;
		}

		default:
			FATAL(true, ("saveToByteVector(): failed, unsupported byte count [%d] specified, should be 1 or 2.", byteCount));
	}
}

// ----------------------------------------------------------------------

bool RangedIntCustomizationVariable::restoreFromByteVector(ByteVector const &data, int startIndex, int length)
{
	if (startIndex < 0)
	{
		WARNING(true, ("restoreFromByteVector(): invalid startIndex [%d]", startIndex));
		return false;
	}
	
	switch (length)
	{
		case 1:
		{
			//-- Handle unsigned byte packing.
			
			// Check range.
			if (startIndex >= static_cast<int>(data.size()))
			{
				WARNING(true, ("restoreFromByteVector(): attempting to restore unsigned byte int but not enough data."));
				return false;
			}
			
			// Restore as single unsigned byte.
			setValue(static_cast<int>(data[static_cast<ByteVector::size_type>(startIndex)]));
			break;
		}

		case 2:
		{
			//-- Handle signed int packing.
			// Check range.
			if (startIndex + 1 >= static_cast<int>(data.size()))
			{
				WARNING(true, ("restoreFromByteVector(): attempting to restore short signed int but not enough data."));
				return false;
			}
			
			// Restore as single unsigned byte.
			uint32 const lowByte = static_cast<uint32>(data[static_cast<ByteVector::size_type>(startIndex)]);
			uint32 const hiByte  = static_cast<uint32>(data[static_cast<ByteVector::size_type>(startIndex + 1)]);

			int16 const value = static_cast<int16>((hiByte << 8) | lowByte);
			setValue(value);
			break;
		}

		default:
			FATAL(true, ("saveToByteVector(): failed, unsupported length [%d] specified, should be 1 or 2.", length));
	}

	// Success.
	return true;
}

// ======================================================================

RangedIntCustomizationVariable::RangedIntCustomizationVariable()
:	CustomizationVariable(),
	m_dependentVariable(0),
	m_isDependentVariable(false)
{
}

// ======================================================================

/**
* This function computes the normalized value.
* @return a number in [0.0,1.0]
*/

float RangedIntCustomizationVariable::getNormalizedFloatValue() const
{
	int low = 0, high = 0;
	getRange (low, high);
	const float range = static_cast<float>(high - low);
	if (range <= 0.0f)
		return 0.0f;

	return clamp (0.0f, static_cast<float>(getValue () - low) / range, 1.0f);
}

//----------------------------------------------------------------------

/**
* This function sets the normalized value
* @param value a number in [0.0,1.0]
*/

void  RangedIntCustomizationVariable::setNormalizedFloatValue(float value)
{
	int low = 0, high = 0;
	getRange (low, high);
	const float range = static_cast<float>(high - low);
	setValue (std::min (low + static_cast<int>(value * range), (high - 1)));
}

//----------------------------------------------------------------------

int RangedIntCustomizationVariable::getRangeSize() const
{
	int low  = 0;
	int high = 0;
	getRange (low, high);
	
	return high - low;
}

// ======================================================================
void RangedIntCustomizationVariable::setDependentVariable(const std::string &variablePathName)
{
	if(getOwner())
	{
		RangedIntCustomizationVariable * const var = safe_cast<RangedIntCustomizationVariable *>(getOwner()->findVariable(variablePathName));
		if(var)
		{
			m_dependentVariable = var;
			var->setIsDependentVariable(true);
		}
	}
}

// ======================================================================
void RangedIntCustomizationVariable::setIsDependentVariable(bool const isDependentVariable)
{
	m_isDependentVariable = isDependentVariable;
}

// ======================================================================
bool RangedIntCustomizationVariable::getIsDependentVariable() const
{
	return m_isDependentVariable;
}

// ======================================================================

bool RangedIntCustomizationVariable::setValue(int value)
{
	if(m_dependentVariable)
	{
		if (!m_dependentVariable->setValue(value))
			return false;
	}

	return true;
}

//----------------------------------------------------------------------
