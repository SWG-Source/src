// ======================================================================
//
// PatrolPathNodeProperty.cpp
// Copyright 2001 Sony Online Entertainment Inc.
// All Rights Reserved.
//
// ======================================================================

#include "serverGame/FirstServerGame.h"
#include "serverGame/PatrolPathNodeProperty.h"
#include "serverGame/ServerObject.h"

#include <set>


// ======================================================================

PropertyId PatrolPathNodeProperty::getClassPropertyId()
{
//	PropertyId id = LabelHash::hashLabel( "Property", "PatrolPathNode" );
//	DEBUG_REPORT_LOG(true, ("PatrolPathNodeProperty::getClassPropertyId id = %lx\n", id));
//	return id;
	return PROPERTY_HASH(PatrolPathNode, 0xef800a48);
}

// ======================================================================

PatrolPathNodeProperty::PatrolPathNodeProperty(Object& thisObject) : 
Property(getClassPropertyId(), thisObject),
m_roots(new std::set<CachedNetworkId>)
{
}

//-------------------------------------------------------------------------

PatrolPathNodeProperty::~PatrolPathNodeProperty()
{
	delete m_roots;
	m_roots = nullptr;
}

//-------------------------------------------------------------------------

void PatrolPathNodeProperty::addRoot(const ServerObject & root)
{
	m_roots->insert(CachedNetworkId(root));
}

//-------------------------------------------------------------------------

bool PatrolPathNodeProperty::hasRoot(const ServerObject & root) const
{
	for (std::set<CachedNetworkId>::const_iterator i = m_roots->begin(); i != m_roots->end(); ++i)
	{
		if (*i == root.getNetworkId())
			return true;
	}
	return false;
}

// ======================================================================
