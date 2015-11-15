// ======================================================================
//
// SlotId.cpp
// Copyright 2001 Sony Online Entertainment Inc.
// All Rights Reserved.
//
// ======================================================================

#include "sharedObject/FirstSharedObject.h"
#include "sharedObject/SlotId.h"

// ======================================================================

const SlotId SlotId::invalid(-1);

// ======================================================================

SlotId::SlotId() :
	m_slot(-1)
{
}

// ----------------------------------------------------------------------

SlotId::SlotId(int slot) : 
	m_slot(slot)
{
}

// ----------------------------------------------------------------------

SlotId::~SlotId()
{
}

// ----------------------------------------------------------------------

SlotId& SlotId::operator=(const SlotId& rhs)
{
	if ((&rhs == this) || (rhs == *this))
		return *this;
	m_slot = rhs.getSlotId();
	return *this;
}

// ======================================================================
