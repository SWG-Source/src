// ======================================================================
//
// WearableEntry.cpp
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#include "sharedGame/FirstSharedGame.h"
#include "sharedGame/WearableEntry.h"

#include "sharedFoundation/NetworkIdArchive.h"


// ======================================================================

WearableEntry::WearableEntry(const std::string& appearance, int arrangement, const NetworkId& id, int sharedTemplate) :
		m_appearanceString(appearance),
		m_arrangement(arrangement),
		m_networkId(id),
		m_objectTemplate(sharedTemplate),
		m_weaponSharedBaselines(),
		m_weaponSharedNpBaselines()
{
	
}

//------------------------------------------------------------------------------------------

WearableEntry::WearableEntry(const std::string& appearance, int arrangement, const NetworkId& id, int sharedTemplate, const BaselinesMessage * weaponSharedBaselines, const BaselinesMessage * weaponSharedNpBaselines) :
		m_appearanceString(appearance),
		m_arrangement(arrangement),
		m_networkId(id),
		m_objectTemplate(sharedTemplate),
		m_weaponSharedBaselines(weaponSharedBaselines),
		m_weaponSharedNpBaselines(weaponSharedNpBaselines)
{
}

//------------------------------------------------------------------------------------------

WearableEntry::WearableEntry() :
		m_appearanceString(""),
		m_arrangement(-1),
		m_networkId(NetworkId::cms_invalid),
		m_objectTemplate(0),
		m_weaponSharedBaselines(),
		m_weaponSharedNpBaselines()
{
	
}

//------------------------------------------------------------------------------------------

WearableEntry::WearableEntry(const WearableEntry& rhs) :
		m_appearanceString(rhs.m_appearanceString),
		m_arrangement(rhs.m_arrangement),
		m_networkId(rhs.m_networkId),
		m_objectTemplate(rhs.m_objectTemplate),
		m_weaponSharedBaselines(rhs.m_weaponSharedBaselines),
		m_weaponSharedNpBaselines(rhs.m_weaponSharedNpBaselines)
{
	
}

//------------------------------------------------------------------------------------------

WearableEntry& WearableEntry::operator=(const WearableEntry& rhs)
{
	m_appearanceString = rhs.m_appearanceString;
	m_arrangement = rhs.m_arrangement;
	m_networkId = rhs.m_networkId;
	m_objectTemplate = rhs.m_objectTemplate;
	m_weaponSharedBaselines = rhs.m_weaponSharedBaselines;
	m_weaponSharedNpBaselines = rhs.m_weaponSharedNpBaselines;

	return *this;
}

//------------------------------------------------------------------------------------------

namespace Archive
{
	void get(ReadIterator & source, WearableEntry & target)
	{
		get(source, target.m_appearanceString);
		get(source, target.m_arrangement);
		get(source, target.m_networkId);
		get(source, target.m_objectTemplate);

		bool isWeapon;
		get(source, isWeapon);
		if (isWeapon)
		{
			target.m_weaponSharedBaselines = std::make_shared<const BaselinesMessage>(source);
			target.m_weaponSharedNpBaselines = std::make_shared<const BaselinesMessage>(source);
		}
	}

	void put(ByteStream & target, const WearableEntry & source)
	{
		put(target, source.m_appearanceString);
		put(target, source.m_arrangement);
		put(target, source.m_networkId);
		put(target, source.m_objectTemplate);

		bool isWeapon = (source.m_weaponSharedBaselines.get() != nullptr);
		put(target, isWeapon);
		if (isWeapon)
		{
			source.m_weaponSharedBaselines->pack(target);
			source.m_weaponSharedNpBaselines->pack(target);
		}
	}
}

// ======================================================================
