// ======================================================================
//
// PaletteColorCustomizationVariable.h
// Copyright 2002 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#ifndef INCLUDED_PaletteColorCustomizationVariable_H
#define INCLUDED_PaletteColorCustomizationVariable_H

// ======================================================================

#include "sharedFoundation/MemoryBlockManagerMacros.h"
#include "sharedObject/RangedIntCustomizationVariable.h"

class MemoryBlockManager;
class PackedArgb;
class PaletteArgb;

// ======================================================================

class PaletteColorCustomizationVariable: public RangedIntCustomizationVariable
{
	MEMORY_BLOCK_MANAGER_INTERFACE_WITH_INSTALL;

public:

	PaletteColorCustomizationVariable(const PaletteArgb *palette, int selectedIndex = 0);
	virtual ~PaletteColorCustomizationVariable();

	virtual int          getValue() const;
	virtual bool         setValue(int value);
	const PackedArgb    &getValueAsColor() const;

	virtual void         getRange(int &minRangeInclusive, int &maxRangeExclusive) const;

	virtual void         writeObjectTemplateExportString(const std::string &variablePathName, ObjectTemplateCustomizationDataWriter &writer) const;

	const PaletteArgb   *fetchPalette() const;

	void                 setClosestColor(const PackedArgb &targetColor);

#ifdef _DEBUG
	virtual std::string  debugToString() const;
#endif

private:

	// disabled
	PaletteColorCustomizationVariable();
	PaletteColorCustomizationVariable(const PaletteColorCustomizationVariable&);
	PaletteColorCustomizationVariable &operator =(const PaletteColorCustomizationVariable&);

private:

	const PaletteArgb *const m_palette;
	int                      m_paletteIndex;

};

// ======================================================================

#endif
