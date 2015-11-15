// ======================================================================
//
// DataResource.cpp - base class for template and other classes.
// copyright 2001 Sony Online Entertainment
//
// ======================================================================

#include "sharedFoundation/FirstSharedFoundation.h"
#include "sharedFoundation/DataResource.h"

#include "sharedFoundation/PersistentCrcString.h"

// ======================================================================

DataResource::DataResource(const char* const filename) :
	m_name(filename, true),
	m_referenceCount(0)
{
}	

// ----------------------------------------------------------------------

DataResource::~DataResource()
{
}	

// ----------------------------------------------------------------------

const char *DataResource::getName() const
{
	return m_name.getString();
}	

// ----------------------------------------------------------------------

int DataResource::getReferenceCount() const
{
	return m_referenceCount;
}	

// ----------------------------------------------------------------------

void DataResource::addReference() const
{
	++m_referenceCount;
}	

// ----------------------------------------------------------------------

void DataResource::releaseReference() const
{
	DEBUG_FATAL(m_referenceCount <= 0, ("DataResource::releaseReference - reference count is already 0\n"));
	if (m_referenceCount > 0 && --m_referenceCount == 0)
		release();
}	

// ----------------------------------------------------------------------

void DataResource::preloadAssets () const
{
}

// ----------------------------------------------------------------------

void DataResource::garbageCollect () const
{
}

// ======================================================================
