// ======================================================================
//
// Dynamics.cpp
// copyright 1999, bootprint entertainment
// copyright 2002, sony online entertainment
//
// ======================================================================

#include "sharedObject/FirstSharedObject.h"
#include "sharedObject/Dynamics.h"

#include "sharedObject/AlterResult.h"

// ======================================================================
// Construct a new dynamics
//
// Remarks:
//
//   This routine does nothing, but is provided for interface reasons for
//   this base class.

Dynamics::Dynamics (Object* const owner) :
	m_owner (owner)
{
}

// ----------------------------------------------------------------------
/**
 * Destroy a new dynamics.
 * 
 * This routine does nothing, but is provided for interface reasons for
 * this base class.
 */

Dynamics::~Dynamics ()
{
	m_owner = 0;
}

// ----------------------------------------------------------------------
/**
 * Alter a dynamics.
 * 
 * This routine does nothing, but is provided for interface reasons for
 * this base class.
 * 
 * @param time  The amount of time elapse since the last alter
 */

float Dynamics::alter (float /*time*/)
{
	// Indicate there's no reason to alter based on functionality in this class.
	return AlterResult::cms_keepNoAlter;
}

// ======================================================================
