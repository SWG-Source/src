// ======================================================================
//
// SlotIdManager.cpp
// Copyright 2001, 2002 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#include "sharedObject/FirstSharedObject.h"
#include "sharedObject/SlotIdManager.h"

#include "sharedFile/Iff.h"
#include "sharedFoundation/ExitChain.h"
#include "sharedFoundation/PointerDeleter.h"
#include "sharedFoundation/PersistentCrcString.h"
#include "sharedFoundation/Tag.h"
#include "sharedObject/SlotId.h"

#include <algorithm>
#include <string>
#include <vector>
#include <map>

// ======================================================================

namespace SlotIdManagerNamespace
{
	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	class Slot
	{
	public:

		struct LessSlotNameComparator
		{
		public:

			bool operator ()(const Slot *lhs, const Slot *rhs) const;
			bool operator ()(const CrcString *lhs, const Slot *rhs) const;
			bool operator ()(const Slot *lhs, const CrcString *rhs) const;

		};

	public:

		Slot(const char *slotName, bool isPlayerModifiable, bool isAppearanceRelated, char const *hardpointName, uint16 combatBone, bool observeWithParent, bool exposeWithParent);

		CrcString const  &getSlotName() const;
		bool              isPlayerModifiable() const;
		bool              isAppearanceRelated() const;
		CrcString const  &getHardpointName() const;
		uint16            getCombatBone() const;
		bool              getObserveWithParent() const;
		bool              getExposeWithParent() const;

	private:

		// disabled
		Slot();
		Slot(const Slot&);             //lint -esym(754, Slot::Slot)      // defensive hiding
		Slot &operator =(const Slot&); //lint -esym(754, Slot::operator=) // defensive hiding

	private:

		PersistentCrcString  m_slotName;
		bool                 m_isPlayerModifiable;
		bool                 m_isAppearanceRelated;
		PersistentCrcString  m_hardpointName;
		uint16               m_combatBone;
		bool                 m_observeWithParent;
		bool                 m_exposeWithParent;
	};

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	typedef std::vector<Slot*>             SlotVector;
	typedef std::multimap<uint32, Slot* >  SlotBoneMap;
	
	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	bool                               s_installed;

	SlotVector                         s_slots;
	SlotBoneMap                        s_slotsSortedByBone;
	SlotIdManager::AnythingSlotVector  s_slotsThatHoldAnything;
}

using namespace SlotIdManagerNamespace;

// ======================================================================
// class SlotIdManager::Slot
// ======================================================================

SlotIdManagerNamespace::Slot::Slot(const char *slotName, bool newIsPlayerModifiable, bool newIsAppearanceRelated, char const *hardpointName, uint16 combatBone, bool observeWithParent, bool exposeWithParent) :
	m_slotName(slotName, true),
	m_isPlayerModifiable(newIsPlayerModifiable),
	m_isAppearanceRelated(newIsAppearanceRelated),
	m_hardpointName(hardpointName, true),
	m_combatBone(combatBone),
	m_observeWithParent(observeWithParent),
	m_exposeWithParent(exposeWithParent)
{
}

// ----------------------------------------------------------------------

inline CrcString const &SlotIdManagerNamespace::Slot::getSlotName() const
{
	return m_slotName;
}

// ----------------------------------------------------------------------

inline bool SlotIdManagerNamespace::Slot::isPlayerModifiable() const
{
	return m_isPlayerModifiable;
}

// ----------------------------------------------------------------------

inline bool SlotIdManagerNamespace::Slot::isAppearanceRelated() const
{
	return m_isAppearanceRelated;
}

// ----------------------------------------------------------------------

inline CrcString const &SlotIdManagerNamespace::Slot::getHardpointName() const
{
	return m_hardpointName;
}

// ----------------------------------------------------------------------

inline uint16 SlotIdManagerNamespace::Slot::getCombatBone() const
{
	return m_combatBone;
}

// ----------------------------------------------------------------------

inline bool SlotIdManagerNamespace::Slot::getObserveWithParent() const
{
	return m_observeWithParent;
}

// ----------------------------------------------------------------------

inline bool SlotIdManagerNamespace::Slot::getExposeWithParent() const
{
	return m_exposeWithParent;
}

// ======================================================================
// struct SlotIdManager::Slot::LessSlotNameComparator
// ======================================================================

inline bool SlotIdManagerNamespace::Slot::LessSlotNameComparator::operator ()(const Slot *lhs, const Slot *rhs) const
{
	NOT_NULL(lhs);
	NOT_NULL(rhs);
	return lhs->getSlotName() < rhs->getSlotName();
}

// ----------------------------------------------------------------------

inline bool SlotIdManagerNamespace::Slot::LessSlotNameComparator::operator ()(const CrcString *lhs, const Slot *rhs) const
{
	NOT_NULL(lhs);
	NOT_NULL(rhs);
	return *lhs < rhs->getSlotName();
}

// ----------------------------------------------------------------------

inline bool SlotIdManagerNamespace::Slot::LessSlotNameComparator::operator ()(const Slot *lhs, const CrcString *rhs) const
{
	NOT_NULL(lhs);
	NOT_NULL(rhs);
	return lhs->getSlotName() < *rhs;
}

// ======================================================================
/**
 * Install the SlotIdManager.
 *
 * Clients should specify true for loadHardpointNameData.  Servers will
 * not need this Appearance-related data loaded.
 *
 * @param filename               The name of the file containing the valid slot data.
 * @param loadHardpointNameData  If true, hardpoint names associated with slots
 *                               will be loaded.
 */

void SlotIdManager::install(const std::string &filename, bool loadHardpointNameData)
{
	DEBUG_FATAL(s_installed, ("SlotIdManager already installed"));

	//-- load valid slot names from file
	Iff iff(filename.c_str());

	const Tag version = iff.getCurrentName();
	switch (version)
	{
		case TAG_0006:
			load_0006(iff, loadHardpointNameData);
			break;

		default:
			{
				char buffer[5];
				ConvertTagToString(version, buffer);
				FATAL(true, ("unsupported SlotIdManager file format [%s]", buffer));
			}
	}

	s_installed = true;
	ExitChain::add(remove, "SlotIdManager");
}

// ----------------------------------------------------------------------

bool SlotIdManager::isInstalled()
{
	return s_installed;
}

// ----------------------------------------------------------------------
/**
 * Retrieve a SlotId instance for the Slot corresponding to the given
 * slot name.
 *
 * If the specified slot name does not exist, the returned SlotId will be 
 * equivalent to SlotId::invalid.
 *
 * @param slotName  the name of the slot to lookup.
 *
 * @return  a SlotId instance that may be used in future calls to retrieve
 *          information regarding the slot.  Also used by the SlottedContainer.
 */

SlotId SlotIdManager::findSlotId(CrcString const &slotName)
{
	DEBUG_FATAL(!s_installed, ("SlotIdManager not installed"));

	//lint -e18 // error, redeclared template // lint appears to be confused by std::equal_range and its return type
	std::pair<SlotVector::iterator, SlotVector::iterator> findResult = std::equal_range(s_slots.begin(), s_slots.end(), &slotName, Slot::LessSlotNameComparator()); //lint !e64 // type mismatch // I think Lint is confused.
	if (findResult.first == findResult.second)
	{
		// return invalid: slot not found.
		return SlotId::invalid;
	}
	else
	{
		// construct a SlotId from the index of the entry just found.
		const int foundIndex = static_cast<int>(std::distance(s_slots.begin(), findResult.first));
		return SlotId(foundIndex);
	}
}

// ----------------------------------------------------------------------
/**
 * Retrieves the slots ids associated with a given combat skeleton bone.
 *
 * @param bone		the combat skeleton bone id
 * @param slots		vector to be filled in with SlotIds
 */
void SlotIdManager::findSlotIdsForCombatBone(uint32 bone, std::vector<SlotId> & slots)
{
	DEBUG_FATAL(!s_installed, ("SlotIdManager not installed"));

	SlotBoneMap::const_iterator low = s_slotsSortedByBone.lower_bound(bone);
	if (low != s_slotsSortedByBone.end())
	{
		SlotBoneMap::const_iterator high = s_slotsSortedByBone.upper_bound(bone);
		SlotBoneMap::const_iterator i;
		for (i = low; i != high; ++i)
		{
			slots.push_back(findSlotId((*i).second->getSlotName()));
		}
	}
}

// ----------------------------------------------------------------------
/**
 * Retrieve the name of a slot given the slot's SlotId.
 *
 * @param slotId  a SlotId instance for the slot under question.
 *
 * @return  the name of the slot.
 */

const CrcString &SlotIdManager::getSlotName(const SlotId &slotId)
{
	DEBUG_FATAL(!s_installed, ("SlotIdManager not installed"));

	//-- return a zero-length string for SlotId::invalid.
	if (slotId == SlotId::invalid)
		return PersistentCrcString::empty;

	//-- retrieve the specified slot
	VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE(0, slotId.getSlotId(), static_cast<int>(s_slots.size()));
	const Slot *const slot = s_slots[static_cast<size_t>(slotId.getSlotId())];
	NOT_NULL(slot);

	return slot->getSlotName();
}
// ----------------------------------------------------------------------
/**
 * Get a list of slots that can hold any item regardless of arrangement.
 * Used by SlottedContainmentProperty
 *
 * @return a vector of SlotIds that hold anything.
 */
const SlotIdManager::AnythingSlotVector& SlotIdManager::getSlotsThatHoldAnything()
{
	DEBUG_FATAL(!s_installed, ("SlotIdManager not installed"));
	return s_slotsThatHoldAnything;
}

// ----------------------------------------------------------------------
/**
 * Retrieve whether the given slot is allowed to be modified directly by the
 * player.
 *
 * @param slotId  a SlotId instance for the slot under question.
 *
 * @return  true if the player may modify a slot directly, false otherwise.
 */

bool SlotIdManager::isSlotPlayerModifiable(const SlotId &slotId)
{
	DEBUG_FATAL(!s_installed, ("SlotIdManager not installed"));

	//-- return a zero-length string for SlotId::invalid.
	if (slotId == SlotId::invalid)
		return false;

	//-- retrieve the specified slot
	VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE(0, slotId.getSlotId(), static_cast<int>(s_slots.size()));
	const Slot *const slot = s_slots[static_cast<size_t>(slotId.getSlotId())];
	NOT_NULL(slot);

	return slot->isPlayerModifiable();
}

// ----------------------------------------------------------------------
/**
 * Retrieve whether the given slot corresponds to an appearance slot with
 * a hardpoint.
 *
 * If the slot can have something put in it that directly affects what you 
 * see on the client, this will return true.  If this returns false,
 * getSlotHardpointName() will return a nullptr string.
 *
 * @param slotId  a SlotId instance for the slot under question.
 *
 * @return  true if the given slot corresponds to an appearance slot with
 *          a hardpoint, false otherwise.
 */

bool SlotIdManager::isSlotAppearanceRelated(const SlotId &slotId)
{
	DEBUG_FATAL(!s_installed, ("SlotIdManager not installed"));

	//-- return a zero-length string for SlotId::invalid.
	if (slotId == SlotId::invalid)
		return false;

	//-- retrieve the specified slot
	VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE(0, slotId.getSlotId(), static_cast<int>(s_slots.size()));
	const Slot *const slot = s_slots[static_cast<size_t>(slotId.getSlotId())];
	NOT_NULL(slot);

	return slot->isAppearanceRelated();
}

// ----------------------------------------------------------------------
/**
 * Retrieve the name of hardpoint associated with the specified slot.
 *
 * The caller should check the result of isSlotAppearanceRelated() before
 * calling this function.  If the slot is not appearance related, this function
 * will always return nullptr.  Also, if the SlotIdManager is installed such that
 * hardpoint names are not loaded (currently the server is loaded this way),
 * the specified hardpoint name will return nullptr.
 *
 * @param slotId  a SlotId instance for the slot under question.
 *
 * @return  the name of the hardpoint associated with the specified slot.
 */

CrcString const &SlotIdManager::getSlotHardpointName(SlotId const &slotId)
{
	DEBUG_FATAL(!s_installed, ("SlotIdManager not installed"));

	//-- return a zero-length string for SlotId::invalid.
	if (slotId == SlotId::invalid)
		return PersistentCrcString::empty;

	//-- retrieve the specified slot
	VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE(0, slotId.getSlotId(), static_cast<int>(s_slots.size()));
	const Slot *const slot = s_slots[static_cast<size_t>(slotId.getSlotId())];
	NOT_NULL(slot);

	return slot->getHardpointName();
}

// ----------------------------------------------------------------------
/**
 * Retrieve whether the slot should have its contents observed when
 * the parent is observed.
 *
 * Here we use observation to mean the concept that a particular object
 * is known and visible to a client.
 *
 * Example usage: the rider slot on a mountable creature is filled with
 * the rider when the rider gets on the mount.  We need all clients
 * that observe the mount (i.e. the parent) to observe the player.
 */

bool SlotIdManager::getSlotObserveWithParent(const SlotId &slotId)
{
	DEBUG_FATAL(!s_installed, ("SlotIdManager not installed"));

	if (slotId == SlotId::invalid)
		return false;

	//-- retrieve the specified slot
	VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE(0, slotId.getSlotId(), static_cast<int>(s_slots.size()));
	const Slot *const slot = s_slots[static_cast<size_t>(slotId.getSlotId())];
	NOT_NULL(slot);

	return slot->getObserveWithParent();
}

// ----------------------------------------------------------------------
/**
 * Retrieve whether the slot should have its contents exposed to the when
 * world when the parent is exposed to the world.
 *
 * Example usage: the rider slot on a mountable creature keeps its rider
 * in the world.
 */
bool SlotIdManager::getSlotExposeWithParent(const SlotId &slotId)
{
	DEBUG_FATAL(!s_installed, ("SlotIdManager not installed"));

	if (slotId == SlotId::invalid)
		return false;

	//-- retrieve the specified slot
	VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE(0, slotId.getSlotId(), static_cast<int>(s_slots.size()));
	const Slot *const slot = s_slots[static_cast<size_t>(slotId.getSlotId())];
	NOT_NULL(slot);

	return slot->getExposeWithParent();
}

// ----------------------------------------------------------------------
/**
 * Retrieve the number of valid, unique slots supported by the system.
 *
 * @return  the number of valid, unique slots supported by the system.
 */

int SlotIdManager::getSlotCount()
{
	DEBUG_FATAL(!s_installed, ("SlotIdManager not installed"));
	return static_cast<int>(s_slots.size());
}

// ----------------------------------------------------------------------
/**
 * Retrieve a SlotId for the specified slot index.
 *
 * @param index  must be in the range 0 .. getSlotCount() - 1, inclusive.
 *
 * @return  a SlotId referencing the index'th slot known to the system.
 */

SlotId SlotIdManager::getSlotIdByIndex(int index)
{
	DEBUG_FATAL(!s_installed, ("SlotIdManager not installed"));
	VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE(0, index, getSlotCount());

	return SlotId(index);
}

// ======================================================================

void SlotIdManager::remove()
{
	DEBUG_FATAL(!s_installed, ("SlotIdManager not installed"));

	AnythingSlotVector().swap(s_slotsThatHoldAnything);
	SlotBoneMap().swap(s_slotsSortedByBone);

	std::for_each(s_slots.begin(), s_slots.end(), PointerDeleter());
	SlotVector().swap(s_slots);
}

//------------------------------------------------------------------------

void SlotIdManager::load_0006(Iff &iff, bool loadHardpointNameData)
{
	//-- load the Slot data
	iff.enterForm(TAG_0006);
		iff.enterChunk(TAG_DATA);
		{
			char slotName[512];
			char hardpointName[512];

			// keep loading entries until done.  we don't include count here
			// because that would prove error prone for the maintainer of the
			// miff data.
			while (iff.getChunkLengthLeft())
			{
				//-- read the slot name
				iff.read_string(slotName, sizeof(slotName) - 1);
				slotName[sizeof(slotName) - 1] = '\0';

				//-- read flags
				const bool canAcceptAnyItem    = (iff.read_uint8() != 0);
				const bool isPlayerModifiable  = (iff.read_uint8() != 0);
				const bool isAppearanceRelated = (iff.read_uint8() != 0);

				iff.read_string(hardpointName, sizeof(hardpointName) - 1);
				hardpointName[sizeof(hardpointName) - 1] = '\0';

				//-- read the combat skeleton "bone" id
				const uint16 combatBone = iff.read_uint16();

				//-- get the slot's observation characteristics.
				bool const observeWithParent = (iff.read_uint8() != 0);
				//-- get the slot's world expose characteristics.
				bool const exposeWithParent = (iff.read_uint8() != 0);

				//-- construct the slot
				Slot * slot = new Slot(slotName, isPlayerModifiable, isAppearanceRelated, loadHardpointNameData ? hardpointName : "", combatBone, observeWithParent, exposeWithParent);
				s_slots.push_back(slot);

				if (canAcceptAnyItem)
				{
					std::string slotString = slotName;
					s_slotsThatHoldAnything.push_back(slotString);
				}

				//-- add the slot bone list for every bone it's associated with
				for (uint16 mask = 0x8000; mask != 0; mask >>= 1)
				{
					if (mask & combatBone)
						IGNORE_RETURN(s_slotsSortedByBone.insert(SlotBoneMap::value_type(mask & combatBone, slot)));
				}
			}
		}
		iff.exitChunk(TAG_DATA);
	iff.exitForm(TAG_0006);

	//-- sort the Slot data
	std::sort(s_slots.begin(), s_slots.end(), Slot::LessSlotNameComparator());
}

// ======================================================================

