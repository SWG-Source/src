// ======================================================================
//
// PatrolPathNodeProperty.h
// Copyright 2001 Sony Online Entertainment Inc.
// All Rights Reserved.
//
// ======================================================================

#ifndef INCLUDED_PatrolPathNodeProperty_H
#define INCLUDED_PatrolPathNodeProperty_H

// ======================================================================

#include "sharedObject/CachedNetworkId.h"
#include "sharedObject/Property.h"


// ======================================================================

class PatrolPathNodeProperty : public Property
{
public:

	static PropertyId getClassPropertyId();

public:
	PatrolPathNodeProperty(Object& thisObject);
	~PatrolPathNodeProperty();

	void                                 addRoot(const ServerObject & root);
	const std::set<CachedNetworkId> & getRoots() const;
	bool                                 hasRoot(const ServerObject & root) const;

private:
	
	std::set<CachedNetworkId> * m_roots;

private:
	PatrolPathNodeProperty();
	PatrolPathNodeProperty(const PatrolPathNodeProperty&);
	PatrolPathNodeProperty& operator= (const PatrolPathNodeProperty&);
};


// ======================================================================

inline const std::set<CachedNetworkId> & PatrolPathNodeProperty::getRoots() const
{
	return *m_roots;
}


// ======================================================================


#endif

