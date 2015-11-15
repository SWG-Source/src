// ======================================================================
//
// PaletteColorCustomizationVariable.cpp
// Copyright 2002 Sony Online Entertainment Inc.
// All Rights Reserved.
//
// ======================================================================

#include "sharedObject/FirstSharedObject.h"
#include "sharedObject/PaletteColorCustomizationVariable.h"

#include "sharedFoundation/ExitChain.h"
#include "sharedFoundation/MemoryBlockManager.h"
#include "sharedMath/PackedArgb.h"
#include "sharedMath/PaletteArgb.h"
#include "sharedObject/ObjectTemplateCustomizationDataWriter.h"

#include <string>
#include <cstdio>

// ======================================================================

MEMORY_BLOCK_MANAGER_IMPLEMENTATION_WITH_INSTALL(PaletteColorCustomizationVariable, true, 0, 0, 0);

// ======================================================================

PaletteColorCustomizationVariable::PaletteColorCustomizationVariable(const PaletteArgb *palette, int selectedIndex)
:	RangedIntCustomizationVariable(),
	m_palette(palette),
	m_paletteIndex(selectedIndex)
{
	NOT_NULL(palette);
	palette->fetch();

	int const paletteEntryCount = m_palette->getEntryCount();
	if ((m_paletteIndex < 0) || (m_paletteIndex >= paletteEntryCount))
	{
		DEBUG_WARNING(true, ("Initializing palette var for palette=[%s] with out-of-range value [%d], valid range is [0..%d], defaulting to [%d].", m_palette->getName().getString(), m_paletteIndex, paletteEntryCount - 1, paletteEntryCount - 1));
		m_paletteIndex = paletteEntryCount - 1;
	}
}

// ----------------------------------------------------------------------

PaletteColorCustomizationVariable::~PaletteColorCustomizationVariable()
{
	//lint -esym(1540, PaletteColorCustomizationVariable::m_palette) // not freed or zero'ed  // yes, but released
	m_palette->release();
}

// ----------------------------------------------------------------------

int PaletteColorCustomizationVariable::getValue() const
{
	return m_paletteIndex;
}

// ----------------------------------------------------------------------

bool PaletteColorCustomizationVariable::setValue(int value)
{
	if ((value >= 0) && (value < m_palette->getEntryCount()))
	{
		// change the value
		m_paletteIndex = value;

		// notify owner CustomizationData about change.
		signalVariableModified();
	}
	else
	{
		DEBUG_WARNING(true, ("attempted to set PaletteColorCustomizationVariable to %d, outside valid range [%d,%d).", value, 0, m_palette->getEntryCount()));
		return false;
	}

	return RangedIntCustomizationVariable::setValue(value);
}

// ----------------------------------------------------------------------

const PackedArgb &PaletteColorCustomizationVariable::getValueAsColor() const
{
	if (!m_palette)
		return PackedArgb::solidMagenta;

	int const usePaletteIndex = clamp(0, m_paletteIndex, m_palette->getEntryCount() - 1);
	bool error = false;
	PackedArgb const & color = m_palette->getEntry(usePaletteIndex, error);

	WARNING(error, ("PaletteColorCustomizationVariable::getValueAsColor error"));
	return color;
}

// ----------------------------------------------------------------------

void PaletteColorCustomizationVariable::getRange(int &minRangeInclusive, int &maxRangeExclusive) const
{
	minRangeInclusive = 0;
	maxRangeExclusive = m_palette->getEntryCount();
}

// ----------------------------------------------------------------------

const PaletteArgb *PaletteColorCustomizationVariable::fetchPalette() const
{
	m_palette->fetch();
	return m_palette;
}

// ----------------------------------------------------------------------

void PaletteColorCustomizationVariable::setClosestColor(const PackedArgb &targetColor)
{
	setValue(m_palette->findClosestMatch(targetColor));
}

// ----------------------------------------------------------------------

void PaletteColorCustomizationVariable::writeObjectTemplateExportString(const std::string &variablePathName, ObjectTemplateCustomizationDataWriter &writer) const
{
#if !(USE_OBJ_TEMPLATE_CUSTOM_VAR_WRITER)
	UNREF(variablePathName);
	UNREF(writer);
#else
	//-- this assumes that the current value is the default value --- if the user changed values at all, they
	//   will not be the artist's built in default.
	writer.addPaletteColorCustomizationVariable(variablePathName, m_palette->getName().getString(), m_paletteIndex);
#endif
}

// ----------------------------------------------------------------------

#ifdef _DEBUG

std::string PaletteColorCustomizationVariable::debugToString() const
{
	char buffer[1024];

	sprintf(buffer, "palette (%s): index %d", m_palette->getName().getString(), m_paletteIndex);
	return std::string(buffer);

}

#endif

// ======================================================================
