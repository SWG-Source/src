// ============================================================================
// 
// ObjectUsabilityManager.h
// Copyright Sony Online Entertainment, Inc.
//
// ============================================================================

#ifndef INCLUDED_ObjectUsabilityManager_H
#define INCLUDED_ObjectUsabilityManager_H

// ============================================================================

#include "sharedGame/SharedCreatureObjectTemplate.h"

// ============================================================================

class CrcString;

//-----------------------------------------------------------------------------

class ObjectUsabilityManager
{
public:

	static void install();

	static bool isWearable(uint32 const wearableSharedTemplateCrc);
	static bool canWear(uint32 const wearableSharedTemplateCrc, SharedCreatureObjectTemplate::Species const species, SharedCreatureObjectTemplate::Gender const gender);
	static bool canWear(uint32 const wearableSharedTemplateCrc, int const species, int const gender);

private:

	static void remove();
};

// ============================================================================

#endif // INCLUDED_ObjectUsabilityManager_H
