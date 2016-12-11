// ======================================================================
//
// PatrolPathRootProperty.h
// Copyright 2001 Sony Online Entertainment Inc.
// All Rights Reserved.
//
// ======================================================================

#ifndef INCLUDED_PatrolPathRootProperty_H
#define INCLUDED_PatrolPathRootProperty_H

// ======================================================================

#include "sharedFoundation/Watcher.h"
#include "sharedObject/Property.h"


class ServerObject;


// ======================================================================

class PatrolPathRootProperty : public Property
{
public:

	static PropertyId getClassPropertyId();

public:
	PatrolPathRootProperty(Object& thisObject);
	~PatrolPathRootProperty();

	int getObserverCount() const;
	void incrementObserverCount();
	void decrementObserverCount();

	void addPatrollingObject(const ServerObject & ai);
	void removePatrollingObject(const ServerObject & ai);
	const std::set<ConstWatcher<ServerObject> > & getPatrollingObjects() const;

private:
	
	int                                        m_observerCount;
	std::set<ConstWatcher<ServerObject> > * m_patrollers;

private:
	PatrolPathRootProperty();
	PatrolPathRootProperty(const PatrolPathRootProperty&);
	PatrolPathRootProperty& operator= (const PatrolPathRootProperty&);
};


// ======================================================================

inline int PatrolPathRootProperty::getObserverCount() const
{
	return m_observerCount;
}

inline void PatrolPathRootProperty::incrementObserverCount()
{
	++m_observerCount;
}

inline void PatrolPathRootProperty::decrementObserverCount()
{
	--m_observerCount;
}


// ======================================================================


#endif

