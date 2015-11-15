//===================================================================
//
// FloraManager.cpp
// asommers
//
// copyright 2000, verant interactive
//
//===================================================================

#include "sharedTerrain/FirstSharedTerrain.h"
#include "sharedTerrain/FloraManager.h"

//===================================================================

namespace FloraManagerNamespace
{
	const Object* ms_referenceObject;
}

using namespace FloraManagerNamespace;

//===================================================================

const Object* FloraManager::getReferenceObject ()
{
	return ms_referenceObject;
}

//-------------------------------------------------------------------

void FloraManager::setReferenceObject (const Object* const referenceObject)
{
	ms_referenceObject = referenceObject;
}

//===================================================================

FloraManager::FloraManager ()
{
}

//-------------------------------------------------------------------

FloraManager::~FloraManager ()
{
}

//===================================================================
