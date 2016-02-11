// ======================================================================
//
// ImmediateDeleteCustomPersistStep.cpp
// copyright (c) 2003 Sony Online Entertainment
//
// ======================================================================

#include "serverDatabase/FirstServerDatabase.h"
#include "serverDatabase/ImmediateDeleteCustomPersistStep.h"

#include "serverDatabase/LazyDeleter.h"
#include <set>

// ======================================================================

ImmediateDeleteCustomPersistStep::ImmediateDeleteCustomPersistStep() : 
		m_objects(new std::set<NetworkId>)
{
}

// ----------------------------------------------------------------------

ImmediateDeleteCustomPersistStep::~ImmediateDeleteCustomPersistStep()
{
	delete m_objects;
	m_objects = nullptr;
}

// ----------------------------------------------------------------------

void ImmediateDeleteCustomPersistStep::addObject(const NetworkId &deletedObject)
{
	m_objects->insert(deletedObject);
}

// ----------------------------------------------------------------------

/**
 * After the object is flagged for deletion and the rest of the data from the snapshot has
 * been saved, give the immediate delete objects to the LazyDeleter.
 */
void ImmediateDeleteCustomPersistStep::onComplete()
{
	for (std::set<NetworkId>::const_iterator i=m_objects->begin(); i!=m_objects->end(); ++i)
		LazyDeleter::getInstance().addObject(*i);
}

// ----------------------------------------------------------------------

bool ImmediateDeleteCustomPersistStep::beforePersist(DB::Session *)
{
	return true;
}

// ----------------------------------------------------------------------

bool ImmediateDeleteCustomPersistStep::afterPersist(DB::Session *)
{
	return true;
}

// ======================================================================
