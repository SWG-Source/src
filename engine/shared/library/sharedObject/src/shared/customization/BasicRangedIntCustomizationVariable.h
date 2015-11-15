// ======================================================================
//
// BasicRangedIntCustomizationVariable.h
// Copyright 2002, 2003 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#ifndef INCLUDED_BasicRangedIntCustomizationVariable_H
#define INCLUDED_BasicRangedIntCustomizationVariable_H

// ======================================================================

#include "sharedFoundation/MemoryBlockManagerMacros.h"
#include "sharedObject/RangedIntCustomizationVariable.h"

class MemoryBlockManager;

// ======================================================================

class BasicRangedIntCustomizationVariable: public RangedIntCustomizationVariable
{
	MEMORY_BLOCK_MANAGER_INTERFACE_WITH_INSTALL;

public:

	BasicRangedIntCustomizationVariable(int minRangeInclusive, int value, int maxRangeExclusive);

	virtual int          getValue() const;
	virtual bool         setValue(int value);

	virtual void         getRange(int &minRangeInclusive, int &maxRangeExclusive) const;

	virtual void         writeObjectTemplateExportString(const std::string &variablePathName, ObjectTemplateCustomizationDataWriter &writer) const;

#ifdef _DEBUG
	virtual std::string  debugToString() const;
#endif

private:

	// disabled
	BasicRangedIntCustomizationVariable();
	BasicRangedIntCustomizationVariable(const BasicRangedIntCustomizationVariable&);
	BasicRangedIntCustomizationVariable &operator =(const BasicRangedIntCustomizationVariable&);

private:

	int  m_value;
	int  m_minRangeInclusive;
	int  m_maxRangeExclusive;

};

// ======================================================================

#endif
