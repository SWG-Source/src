// ======================================================================
//
// DataResource.h - base class for template and other classes.
// copyright 2001 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_DataResource_H
#define INCLUDED_DataResource_H

// ======================================================================

#include "sharedFoundation/PersistentCrcString.h"

// ======================================================================

class DataResource
{
public:

	explicit DataResource(const char* filename);
	virtual ~DataResource() = 0;

	const char           *getName() const;
	const CrcString      &getCrcName() const;

	int                   getReferenceCount() const;
	virtual void          addReference() const;
	virtual void          releaseReference() const;

	virtual void preloadAssets () const;
	virtual void garbageCollect () const;

protected:

	virtual void          release() const = 0;

private:

	DataResource ();
	DataResource(const DataResource &source);
	DataResource &operator =(const DataResource &source);

private:
	
	// resource name
	const PersistentCrcString m_name;            

	// how many times this resource has been loaded
	mutable int               m_referenceCount;  
};

// ----------------------------------------------------------------------

inline const CrcString &DataResource::getCrcName() const
{
	return m_name;
}

// ======================================================================

#endif
