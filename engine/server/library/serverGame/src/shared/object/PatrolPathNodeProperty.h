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
	const stdset<CachedNetworkId>::fwd & getRoots() const;
	bool                                 hasRoot(const ServerObject & root) const;

private:
	
	stdset<CachedNetworkId>::fwd * m_roots;

private:
	PatrolPathNodeProperty();
	PatrolPathNodeProperty(const PatrolPathNodeProperty&);
	PatrolPathNodeProperty& operator= (const PatrolPathNodeProperty&);
};


// ======================================================================

inline const stdset<CachedNetworkId>::fwd & PatrolPathNodeProperty::getRoots() const
{
	return *m_roots;
}


// ======================================================================


#endif

