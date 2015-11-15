// ======================================================================
//
// AuctionItem.h
// copyright (c) 2004 Sony Online Entertainment
//
// (refactorof original code only to the degree to make it work with new)
// (server & database framework ... i.e. none of the logic has changed)
//
// Original Author: Matt Severenson
// Refactored by  : Doug Mellencamp
//
// ======================================================================

#ifndef AuctionItem_h
#define AuctionItem_h

#include "Unicode.h"
#include "sharedFoundation/NetworkId.h"

// ======================================================================

class AuctionItem
{
private:
	AuctionItem();
	AuctionItem(const AuctionItem&);
	AuctionItem& operator= (const AuctionItem&);

protected:
	const NetworkId       m_itemId;
	const int             m_category;
	const int             m_itemTemplateId;
	int                   m_resourceContainerClassCrc; // for resource container, this contains the crc of the resource class
	std::string           m_resourceName; // for resource container, this is the name of the resource (like Iblovris)
	int                   m_resourceNameCrc; // crc for m_resourceName
	const int             m_itemTimer;
	const int             m_nameLength;
	const Unicode::String m_name;
	NetworkId             m_ownerId;
	const int             m_size;

public:
	AuctionItem(const NetworkId & itemId, int category, int itemTemplateId, int itemTimer, int nameLength, const Unicode::String & name, const NetworkId & ownerId, int size);
	~AuctionItem();

	const NetworkId &         GetItemId() const {return m_itemId;}
	int                       GetCategory() const {return m_category;}
	int                       GetItemTemplateId() const {return m_itemTemplateId;}
	int                       GetResourceContainerClassCrc() const {return m_resourceContainerClassCrc;}
	const std::string &       GetResourceName() const {return m_resourceName;}
	int                       GetResourceNameCrc() const {return m_resourceNameCrc;}
	void                      SetResourceContainerClassCrc(int resourceContainerClassCrc) {m_resourceContainerClassCrc = resourceContainerClassCrc;}
	void                      SetResourceName(const std::string & resourceName) {m_resourceName = resourceName;}
	void                      SetResourceNameCrc(int resourceNameCrc) {m_resourceNameCrc = resourceNameCrc;}
	int                       GetItemTimer() const {return m_itemTimer;}
	int                       GetNameLength() const {return m_nameLength;}
	const Unicode::String &   GetName() const {return m_name;}
	const NetworkId &         GetOwnerId() const {return m_ownerId;}
	int                       GetSize() const {return m_size;}

	void                      SetOwnerId(const NetworkId & ownerId) {m_ownerId = ownerId;}

	bool                      IsExpired(int gameTime) const;

	static bool               IsCategoryResourceContainer(int category) {return ((category & 0xffffff00) == 0x00400000);} // SharedObjectTemplate::GOT_resource_container is 0x00400000
};

#endif

// ======================================================================
