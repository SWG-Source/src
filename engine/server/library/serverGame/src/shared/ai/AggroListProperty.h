// ======================================================================
//
// AggroListProperty.h
// Copyright 2005 Sony Online Entertainment Inc.
// All Rights Reserved.
//
// ======================================================================

#ifndef	INCLUDED_AggroListProperty_H
#define	INCLUDED_AggroListProperty_H

#include "Archive/AutoDeltaSet.h"
#include "sharedObject/CachedNetworkId.h"
#include "sharedObject/Property.h"

class TangibleObject;

// ======================================================================
//
// AggroListProperty
//
// ======================================================================

class AggroListProperty : public Property
{
public:

	typedef std::set<CachedNetworkId> TargetList;

	static int getAggroListPropertyResetTime();
	static AggroListProperty * getAggroListProperty(Object & object);
	static PropertyId getClassPropertyId();

public:

	AggroListProperty(TangibleObject & owner);
	~AggroListProperty();

	void addServerNpAutoDeltaVariables(Archive::AutoDeltaByteStream & stream);
	void alter();

	void setAggroDistance(float aggroDistance);
	float getAggroDistance() const;

	void addTarget(NetworkId const & target);
	void removeTarget(NetworkId const & target);
	void clear();

	bool isEmpty() const;
	TargetList const & getTargetList() const;

private:

	TangibleObject & getTangibleOwner();

	AggroListProperty(AggroListProperty const & AggroListProperty);
	bool operator ==(AggroListProperty const & rhs) const;
	bool operator !=(AggroListProperty const & rhs) const;
	AggroListProperty & operator =(AggroListProperty const & rhs);

	Archive::AutoDeltaSet<CachedNetworkId> m_targetList;
	Archive::AutoDeltaVariable<float> m_aggroDistance;
};

// ======================================================================

#endif // INCLUDED_AggroListProperty_H
