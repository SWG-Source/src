// ======================================================================
//
// SlottedContainer.h
// Copyright 2001 Sony Online Entertainment Inc.
// All Rights Reserved.
//
// ======================================================================

#ifndef	INCLUDED_SlottedContainer_H
#define	INCLUDED_SlottedContainer_H

// ======================================================================

#include "sharedObject/Container.h"

// ======================================================================

class SlottedContainer : public Container
{
public:

	static PropertyId getClassPropertyId();

public:

	SlottedContainer(Object& owner, const std::vector<SlotId>& validSlots);
	~SlottedContainer();

	virtual bool                   isContentItemObservedWith(Object const &item) const;
	virtual bool                   isContentItemExposedWith(Object const &item) const;
	virtual bool                   canContentsBeObservedWith() const;

	bool						   add (Object& item, int arrangementIndex, ContainerErrorCode& error);
	bool                           add (Object& item, const SlotId & slotId, ContainerErrorCode& error);
	Container::ContainedItem       getObjectInSlot (const SlotId &slot, ContainerErrorCode& error);
	const Container::ContainedItem getObjectInSlot (const SlotId &slot, ContainerErrorCode& error) const;
	void                           getObjectsForCombatBone(uint32 bone, std::vector<Container::ContainedItem> & objects) const;

	bool                           getFirstUnoccupiedArrangement(const Object& item, int& arrangementIndex, ContainerErrorCode& error) const;
	void                           getValidArrangements(const Object& item, std::vector<int>& returnList, ContainerErrorCode & error, bool returnOnFirst = false, bool unoccupiedArrangementsOnly = false) const;
	bool                           hasSlot (const SlotId &slot) const;
	bool                           isSlotEmpty (const SlotId &slot, ContainerErrorCode& error) const;

	virtual void                   debugPrint(std::string &buffer) const;
	virtual void                   debugLog() const;

	virtual bool                   mayAdd(const Object& item, ContainerErrorCode& error) const;
	bool                           mayAdd(const Object& item, int arrangementIndex, ContainerErrorCode& error) const;
	bool                           mayAdd(const Object& item, const SlotId & slotId, ContainerErrorCode& error) const;
	virtual bool                   remove(Object &item, ContainerErrorCode& error);
	virtual bool                   remove(ContainerIterator &pos, ContainerErrorCode& error);
	void                           removeItemFromSlotOnly(const Object &item);
		
	void                           getSlotIdList(std::vector<SlotId> & slots) const;

	const SlotId &                 findFirstSlotIdForObject (const NetworkId & id) const;

	//might need an addSlot() interface for the loader

	//persistance interface.  Do not call directly.
	virtual int  depersistContents(Object& item);
	void         depersistSlotContents(Object& item, int arrangement);
	virtual bool internalItemRemoved(const Object& item);
	void         updateArrangement(Object& item, int oldArrangement, int newArrangement);

private:

	typedef std::map<SlotId, int>  SlotMap;

private:

	/** shortened find on the map.  Returns -1 if not found.
	*/
	int  find(const SlotId &slot) const;

	bool internalCheckSlottedAdd( Object& item, int arrangementIndex, ContainerErrorCode& error ) const;
	bool internalDoSlottedAdd( Object& item, int position, int arrangementIndex );
	bool internalRemove(const Object& item, int overrideArrangement = -1);

	/** This is the map of slot ids to contents.  It is initialized with -1 values.
	*   Anything greater than or equal to 0 indicates a position in the contents list.
	*/
	SlotMap  *m_slotMap;

	SlottedContainer();
	SlottedContainer(const SlottedContainer&);
	SlottedContainer& operator= (const SlottedContainer&);
};

// ======================================================================

#endif
