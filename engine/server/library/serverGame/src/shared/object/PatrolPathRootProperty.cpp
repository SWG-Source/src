// ======================================================================
//
// PatrolPathRootProperty.cpp
// Copyright 2001 Sony Online Entertainment Inc.
// All Rights Reserved.
//
// ======================================================================

#include "serverGame/FirstServerGame.h"
#include "serverGame/PatrolPathRootProperty.h"
#include "serverGame/ServerObject.h"

#include <set>


// ======================================================================

PropertyId PatrolPathRootProperty::getClassPropertyId()
{
//	PropertyId id = LabelHash::hashLabel( "Property", "PatrolPathRoot" );
//	DEBUG_REPORT_LOG(true, ("PatrolPathRootProperty::getClassPropertyId id = %lx\n", id));
//	return id;
	return PROPERTY_HASH(PatrolPathRoot, 0x2df829a2);
}

// ======================================================================

PatrolPathRootProperty::PatrolPathRootProperty(Object& thisObject) : 
Property(getClassPropertyId(), thisObject),
m_observerCount(0),
m_patrollers(new std::set<ConstWatcher<ServerObject> >)
{
}

//-------------------------------------------------------------------------

PatrolPathRootProperty::~PatrolPathRootProperty()
{
	m_observerCount = 0;

	delete m_patrollers;
	m_patrollers = 0;
}

//-------------------------------------------------------------------------

void PatrolPathRootProperty::addPatrollingObject(const ServerObject & ai)
{
	m_patrollers->insert(ConstWatcher<ServerObject>(&ai));
}

//-------------------------------------------------------------------------

void PatrolPathRootProperty::removePatrollingObject(const ServerObject & ai)
{
	m_patrollers->erase(ConstWatcher<ServerObject>(&ai));
}

//-------------------------------------------------------------------------

const std::set<ConstWatcher<ServerObject> > & PatrolPathRootProperty::getPatrollingObjects() const
{
	return *m_patrollers;
}


// ======================================================================
