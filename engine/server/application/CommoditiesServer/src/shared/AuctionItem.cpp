// ======================================================================
//
// AuctionItem.cpp
// copyright (c) 2004 Sony Online Entertainment
//
// (refactorof original code only to the degree to make it work with new)
// (server & database framework ... i.e. none of the logic has changed)
//
// Original Author: Matt Severenson
// Refactored by  : Doug Mellencamp
//
// ======================================================================

#include "FirstCommodityServer.h"
#include "AuctionItem.h"

// ======================================================================

AuctionItem::AuctionItem(
	const NetworkId & itemId,
	const int category,
	const int itemTemplateId,
	const int itemTimer,
	const int nameLength,
	const Unicode::String & name,
	const NetworkId & ownerId,
	const int size
) :
m_itemId(itemId),
m_category(category),
m_itemTemplateId(itemTemplateId),
m_resourceContainerClassCrc(0),
m_resourceName(),
m_resourceNameCrc(0),
m_itemTimer(itemTimer),
m_nameLength(nameLength),
m_name(name),
m_ownerId(ownerId),
m_size(size)
{
}

// ----------------------------------------------------------------------

AuctionItem::~AuctionItem()
{
}

// ----------------------------------------------------------------------

bool AuctionItem::IsExpired(int gameTime) const
{
	return (gameTime > m_itemTimer);
}

// ======================================================================
