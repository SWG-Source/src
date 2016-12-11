// ======================================================================
//
// SlotIdManager.h
// Copyright 2001, 2002 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#ifndef INCLUDED_SlotIdManager_H
#define INCLUDED_SlotIdManager_H

// ======================================================================

class CrcString;
class Iff;
class SlotId;

// ======================================================================
/**
 *  The SlotIdManager class maps valid slot names to SlotId objects.
 *  For clients, it can also provide the Appearance-related hardpoint names
 *  associated with a given slot.
 */

class SlotIdManager
{
public:

	typedef std::vector<std::string> AnythingSlotVector;

public:

	static void                  install(const std::string &filename, bool loadHardpointNameData);
	static bool                  isInstalled();

	static SlotId                findSlotId(const CrcString &slotName);
	static void                  findSlotIdsForCombatBone(uint32 bone, std::vector<SlotId> & slots);

	static CrcString const      &getSlotName(SlotId const &slotId);
	static bool                  isSlotPlayerModifiable(const SlotId &slotId);
	static bool                  isSlotAppearanceRelated(const SlotId &slotId);
	static CrcString const      &getSlotHardpointName(SlotId const &slotId);
	static bool                  getSlotObserveWithParent(const SlotId &slotId);
	static bool                  getSlotExposeWithParent(const SlotId &slotId);

	static int                   getSlotCount();
	static SlotId                getSlotIdByIndex(int index);

	static const AnythingSlotVector& getSlotsThatHoldAnything();

private:

	static void                  remove();

	static void                  load_0006(Iff &iff, bool loadHardpointNameData);

};

// ======================================================================

#endif
