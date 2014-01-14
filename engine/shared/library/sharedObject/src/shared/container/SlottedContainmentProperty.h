// ======================================================================
//
// SlottedContainmentProperty.h
// Copyright 2001 Sony Online Entertainment Inc.
// All Rights Reserved.
//
// ======================================================================

#ifndef INCLUDED_SlottedContainmentProperty_H
#define INCLUDED_SlottedContainmentProperty_H

// ======================================================================

#include "sharedFoundation/MemoryBlockManagerMacros.h"
#include "sharedObject/Property.h"
#include <vector>

// ======================================================================

class SlotId;

// ======================================================================

class SlottedContainmentProperty : public Property
{
	MEMORY_BLOCK_MANAGER_INTERFACE_WITH_INSTALL;

public:

	static PropertyId getClassPropertyId();

	enum
	{
		SCP_MaxArrangementSize = 0xFFF
	};

	typedef std::vector<SlotId> SlotArrangement;
	typedef std::vector<SlotArrangement> ArrangementList;
	//revisit possibly using iterators
	
public:
	explicit SlottedContainmentProperty(Object& owner);
	~SlottedContainmentProperty();
	
	void      addArrangement(const SlotArrangement &arrangement);

	bool      canManipulateArrangement(int arrangementIndex) const;
	
	int       getBestArrangementForSlot(const SlotId & slotId) const;
	int       getCurrentArrangement() const;
	int       getNumberOfArrangements () const;
	int       getNumberOfSlots (int arrangementIndex) const;

	const SlotArrangement & getSlotArrangement(int arrangementIndex) const;

	SlotId    getSlotId (int arrangementIndex, int slotIndex) const;

	bool      isInAppearanceSlot() const;
	void      setCurrentArrangement(int arrangement, bool local = true);
	
private:
	ArrangementList m_arrangementList;  //This could also be with the object template! Definately not persisted
	int m_currentArrangement;

	SlottedContainmentProperty();
	SlottedContainmentProperty(const SlottedContainmentProperty&);
	SlottedContainmentProperty& operator= (const SlottedContainmentProperty&);
};

// ======================================================================

#endif

