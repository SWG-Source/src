// ======================================================================
//
// CustomizationVariable.h
// Copyright 2002 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#include "sharedObject/FirstSharedObject.h"
#include "sharedObject/CustomizationVariable.h"

#include "sharedObject/CustomizationData.h"

// ======================================================================

CustomizationVariable::~CustomizationVariable()
{
	m_owner = 0;
}

// ======================================================================

CustomizationVariable::CustomizationVariable() :
	m_owner(0)
{
}

// ----------------------------------------------------------------------

void CustomizationVariable::signalVariableModified() const
{
	if (m_owner)
	{
		// tell the owner one of its variables has been modified.
		m_owner->signalModified();
	}
}

// ----------------------------------------------------------------------

void CustomizationVariable::alter(CustomizationData & /* owner */)
{
	// Default does nothing.  Very few of these need to handle alter.
	// Alter is not called every frame --- on client it is only called after
	// processing container changes, and then it only does something for
	// the SlotRuleCustomizationVariable.
}

// ======================================================================
