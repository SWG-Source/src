// ======================================================================
//
// WearableEntry.h
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_WearableEntry_H
#define INCLUDED_WearableEntry_H

// ======================================================================

#include "Archive/AutoByteStream.h"
#include "sharedFoundation/NetworkId.h"
#include "sharedNetworkMessages/BaselinesMessage.h"

#include <memory>


class WearableEntry;

namespace Archive
{
	void get(ReadIterator & source, WearableEntry & target);
	void put(ByteStream & target, const WearableEntry & source);
}

// ======================================================================

class WearableEntry
{
	friend void Archive::get(Archive::ReadIterator & source, WearableEntry & target);
	friend void Archive::put(Archive::ByteStream & target, const WearableEntry & source);

  public:
	WearableEntry(const std::string&, int, const NetworkId&, int);
	WearableEntry(const std::string&, int, const NetworkId&, int, const BaselinesMessage *, const BaselinesMessage *);
	WearableEntry();
	WearableEntry(const WearableEntry&);
	WearableEntry& operator=(const WearableEntry&);
	
	std::string m_appearanceString;
	int         m_arrangement;
	NetworkId   m_networkId;
	int         m_objectTemplate;
	std::shared_ptr<const BaselinesMessage> m_weaponSharedBaselines;
	std::shared_ptr<const BaselinesMessage> m_weaponSharedNpBaselines;

	bool operator==(const WearableEntry&) const;

private:
	void clearWeaponBaseLines();
};

//------------------------------------------------------------------------------------------


inline bool WearableEntry::operator==(const WearableEntry & rhs) const
{
	return this == &rhs || (m_networkId == rhs.m_networkId);
}

// ======================================================================

#endif
