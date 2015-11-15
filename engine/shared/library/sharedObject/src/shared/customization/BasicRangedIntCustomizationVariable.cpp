// ======================================================================
//
// BasicRangedIntCustomizationVariable.cpp
// Copyright 2002, 2003 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#include "sharedObject/FirstSharedObject.h"
#include "sharedObject/BasicRangedIntCustomizationVariable.h"

#include "sharedFoundation/ExitChain.h"
#include "sharedFoundation/MemoryBlockManager.h"
#include "sharedObject/ObjectTemplateCustomizationDataWriter.h"

#include <string>
#include <cstdio>

// ======================================================================

MEMORY_BLOCK_MANAGER_IMPLEMENTATION_WITH_INSTALL(BasicRangedIntCustomizationVariable, true, 0, 0, 0);

// ======================================================================

BasicRangedIntCustomizationVariable::BasicRangedIntCustomizationVariable(int minRangeInclusive, int value, int maxRangeExclusive) :
	RangedIntCustomizationVariable(),
	m_value(value),
	m_minRangeInclusive(minRangeInclusive),
	m_maxRangeExclusive(maxRangeExclusive)
{
}

// ----------------------------------------------------------------------

int BasicRangedIntCustomizationVariable::getValue() const
{
	return m_value;
}

// ----------------------------------------------------------------------

bool BasicRangedIntCustomizationVariable::setValue(int value)
{
	if ((value >= m_minRangeInclusive) && (value < m_maxRangeExclusive))
	{
		// change the value
		m_value = value;

		// notify owner CustomizationData about change.
		signalVariableModified();
	}
	else
	{
		DEBUG_WARNING(true, ("attempted to set BasicRangedIntCustomizationVariable to %d, outside valid range [%d,%d).", value, m_minRangeInclusive, m_maxRangeExclusive));
		return false;
	}

	return RangedIntCustomizationVariable::setValue(value);
}

// ----------------------------------------------------------------------

void BasicRangedIntCustomizationVariable::getRange(int &minRangeInclusive, int &maxRangeExclusive) const
{
	minRangeInclusive = m_minRangeInclusive;
	maxRangeExclusive = m_maxRangeExclusive;
}

// ----------------------------------------------------------------------

void BasicRangedIntCustomizationVariable::writeObjectTemplateExportString(const std::string &variablePathName, ObjectTemplateCustomizationDataWriter &writer) const
{
#if !(USE_OBJ_TEMPLATE_CUSTOM_VAR_WRITER)
	UNREF(variablePathName);
	UNREF(writer);
#else
	//-- this assumes that the current value is the default value --- if the user changed values at all, they
	//   will not be the artist's built in default.
	writer.addRangedIntCustomizationVariable(variablePathName, m_minRangeInclusive, m_value, m_maxRangeExclusive);
#endif
}

// ----------------------------------------------------------------------

#ifdef _DEBUG

std::string BasicRangedIntCustomizationVariable::debugToString() const
{
	char buffer[128];

	sprintf(buffer, "[%d,%d,%d)", m_minRangeInclusive, m_value, m_maxRangeExclusive);
	return std::string(buffer);
}

#endif

// ======================================================================
