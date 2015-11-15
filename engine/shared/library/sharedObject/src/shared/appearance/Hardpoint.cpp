// ======================================================================
//
// Hardpoint.cpp
// Copyright 2000 - 2002 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#include "sharedObject/FirstSharedObject.h"
#include "sharedObject/Hardpoint.h"

// ======================================================================

Hardpoint::Hardpoint (const char* newName, const Transform& newTransform) :
	m_name (newName, false),
	m_transform (newTransform)
{
}

// ----------------------------------------------------------------------

CrcString const& Hardpoint::getName () const
{
	return m_name;
}

// ----------------------------------------------------------------------

Transform const& Hardpoint::getTransform () const
{
	return m_transform;
}

// ======================================================================
