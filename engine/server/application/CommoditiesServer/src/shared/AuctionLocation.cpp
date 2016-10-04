// ======================================================================
//
// AuctionLocation.cpp
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
#include "AuctionLocation.h"
#include "AuctionMarket.h"
#include "DatabaseServerConnection.h"
#include "CommodityServer.h"
#include "serverNetworkMessages/CMUpdateLocationMessage.h"
#include "serverNetworkMessages/AuctionBase.h"
#include "sharedGame/GameObjectTypes.h"

// ======================================================================

namespace AuctionMarketNameSpace
{
	int const cs_statusMask = 0x00FFFFFF;
	int const cs_packedFlag = 0x01000000;

	std::map<NetworkId, Auction *> emptyAuctionList;
}

using namespace AuctionMarketNameSpace;

// ======================================================================

static AuctionMarket::VendorStatusCode nextStatus[AuctionMarket::REMOVED + 1][AuctionMarket::REMOVED + 1];

// ----------------------------------------------------------------------

void AuctionLocation::Initialization()
{
	nextStatus[AuctionMarket::ACTIVE][AuctionMarket::ACTIVE] = AuctionMarket::ACTIVE;
	nextStatus[AuctionMarket::ACTIVE][AuctionMarket::EMPTY] = AuctionMarket::EMPTY;
	nextStatus[AuctionMarket::ACTIVE][AuctionMarket::UNACCESSED] = AuctionMarket::UNACCESSED;
	nextStatus[AuctionMarket::ACTIVE][AuctionMarket::EMPTY_AND_UNACCESSED] = AuctionMarket::EMPTY_AND_UNACCESSED;
	nextStatus[AuctionMarket::ACTIVE][AuctionMarket::ENDANGERED] = AuctionMarket::ACTIVE;
	nextStatus[AuctionMarket::ACTIVE][AuctionMarket::REMOVED] = AuctionMarket::ACTIVE;
	nextStatus[AuctionMarket::EMPTY][AuctionMarket::ACTIVE] = AuctionMarket::ACTIVE;
	nextStatus[AuctionMarket::EMPTY][AuctionMarket::EMPTY] = AuctionMarket::EMPTY;
	nextStatus[AuctionMarket::EMPTY][AuctionMarket::UNACCESSED] = AuctionMarket::EMPTY_AND_UNACCESSED;
	nextStatus[AuctionMarket::EMPTY][AuctionMarket::EMPTY_AND_UNACCESSED] = AuctionMarket::EMPTY_AND_UNACCESSED;
	nextStatus[AuctionMarket::EMPTY][AuctionMarket::ENDANGERED] = AuctionMarket::ENDANGERED;
	nextStatus[AuctionMarket::EMPTY][AuctionMarket::REMOVED] = AuctionMarket::EMPTY;
	nextStatus[AuctionMarket::UNACCESSED][AuctionMarket::ACTIVE] = AuctionMarket::ACTIVE;
	nextStatus[AuctionMarket::UNACCESSED][AuctionMarket::EMPTY] = AuctionMarket::EMPTY_AND_UNACCESSED;
	nextStatus[AuctionMarket::UNACCESSED][AuctionMarket::UNACCESSED] = AuctionMarket::UNACCESSED;
	nextStatus[AuctionMarket::UNACCESSED][AuctionMarket::EMPTY_AND_UNACCESSED] = AuctionMarket::EMPTY_AND_UNACCESSED;
	nextStatus[AuctionMarket::UNACCESSED][AuctionMarket::ENDANGERED] = AuctionMarket::ENDANGERED;
	nextStatus[AuctionMarket::UNACCESSED][AuctionMarket::REMOVED] = AuctionMarket::UNACCESSED;
	nextStatus[AuctionMarket::EMPTY_AND_UNACCESSED][AuctionMarket::ACTIVE] = AuctionMarket::ACTIVE;
	nextStatus[AuctionMarket::EMPTY_AND_UNACCESSED][AuctionMarket::EMPTY] = AuctionMarket::EMPTY_AND_UNACCESSED;
	nextStatus[AuctionMarket::EMPTY_AND_UNACCESSED][AuctionMarket::UNACCESSED] = AuctionMarket::EMPTY_AND_UNACCESSED;
	nextStatus[AuctionMarket::EMPTY_AND_UNACCESSED][AuctionMarket::EMPTY_AND_UNACCESSED] = AuctionMarket::EMPTY_AND_UNACCESSED;
	nextStatus[AuctionMarket::EMPTY_AND_UNACCESSED][AuctionMarket::ENDANGERED] = AuctionMarket::ENDANGERED;
	nextStatus[AuctionMarket::EMPTY_AND_UNACCESSED][AuctionMarket::REMOVED] = AuctionMarket::EMPTY_AND_UNACCESSED;
	nextStatus[AuctionMarket::ENDANGERED][AuctionMarket::ACTIVE] = AuctionMarket::ACTIVE;
	nextStatus[AuctionMarket::ENDANGERED][AuctionMarket::EMPTY] = AuctionMarket::ENDANGERED;
	nextStatus[AuctionMarket::ENDANGERED][AuctionMarket::UNACCESSED] = AuctionMarket::ENDANGERED;
	nextStatus[AuctionMarket::ENDANGERED][AuctionMarket::EMPTY_AND_UNACCESSED] = AuctionMarket::ENDANGERED;
	nextStatus[AuctionMarket::ENDANGERED][AuctionMarket::ENDANGERED] = AuctionMarket::ENDANGERED;
	nextStatus[AuctionMarket::ENDANGERED][AuctionMarket::REMOVED] = AuctionMarket::REMOVED;
	nextStatus[AuctionMarket::REMOVED][AuctionMarket::ACTIVE] = AuctionMarket::REMOVED;
	nextStatus[AuctionMarket::REMOVED][AuctionMarket::EMPTY] = AuctionMarket::REMOVED;
	nextStatus[AuctionMarket::REMOVED][AuctionMarket::UNACCESSED] = AuctionMarket::REMOVED;
	nextStatus[AuctionMarket::REMOVED][AuctionMarket::EMPTY_AND_UNACCESSED] = AuctionMarket::REMOVED;
	nextStatus[AuctionMarket::REMOVED][AuctionMarket::ENDANGERED] = AuctionMarket::REMOVED;
	nextStatus[AuctionMarket::REMOVED][AuctionMarket::REMOVED] = AuctionMarket::REMOVED;
}

// ----------------------------------------------------------------------

AuctionLocation::AuctionLocation(
	const NetworkId & locationId,
	const std::string & location,
	const NetworkId & ownerId,
	int salesTax,
	const NetworkId & salesTaxBankId,
	int emptyDate,
	int lastAccessDate,
	int inactiveDate,
	int status,
	bool searchEnabled,
	int entranceCharge
) :
m_locationId(locationId),
m_ownerId(ownerId),
m_location(location),
m_locationPlanet(),
m_locationRegion(),
m_salesTax(salesTax),
m_salesTaxBankId(salesTaxBankId),
m_emptyDate(emptyDate),
m_lastAccessDate(lastAccessDate),
m_vendorFirstTimerExpiredAuctionDate(0),
m_inactiveDate(inactiveDate),
m_nextUpdateTime(0), // force an immediate update on the auction location
m_status(status),
m_searchEnabled(searchEnabled),
m_entranceCharge(entranceCharge),
m_auctionItemCount(0),
m_auctions(),
m_auctionsIndexedByType(),
m_auctionsIndexedByParentTypeExactMatch(),
m_auctionsIndexedByTemplate(),
m_auctionsResourceContainer(),
m_vendorOffers()
{
	assert(!location.empty());
	assert(GetStatus() >= AuctionMarket::ACTIVE && GetStatus() <= AuctionMarket::REMOVED);

	AuctionMarket::getPlanetAndRegionFromLocationString(m_location, m_locationPlanet, m_locationRegion);
	AuctionMarket::getInstance().addAuctionLocationToLocationIndex(this);

	AuctionMarket::getInstance().AddAuctionLocationToPriorityQueue(*this);
}

// ----------------------------------------------------------------------

AuctionLocation::~AuctionLocation()
{
	AuctionMarket::getInstance().sanityCheckAuctionLocationBeingDestroyed(this);
	AuctionMarket::getInstance().RemoveAuctionLocationFromPriorityQueue(*this);
}

// ----------------------------------------------------------------------

bool AuctionLocation::AddAuction(Auction *auction)
{
	assert(auction != nullptr);
	if (IsVendorMarket() &&
		(!auction->IsActive() || !IsOwner(auction->GetCreatorId())))
	{
//		printf("!!!Offering item to vendor\n");
		m_vendorOffers.insert(std::make_pair(auction->GetItem().GetItemId(), auction));
	}
	else
	{
		m_auctionItemCount += auction->GetItem().GetSize();
		m_auctions.insert(std::make_pair(auction->GetItem().GetItemId(), auction));

		if (AuctionItem::IsCategoryResourceContainer(auction->GetItem().GetCategory()))
		{
			m_auctionsResourceContainer.insert(std::make_pair(auction->GetItem().GetItemId(), auction));
		}
		else if (auction->GetItem().GetCategory() != 0)
		{
			m_auctionsIndexedByType[auction->GetItem().GetCategory()].insert(std::make_pair(auction->GetItem().GetItemId(), auction));

			if (GameObjectTypes::isSubType(auction->GetItem().GetCategory()))
			{
				m_auctionsIndexedByType[GameObjectTypes::getMaskedType(auction->GetItem().GetCategory())].insert(std::make_pair(auction->GetItem().GetItemId(), auction));
			}
			else
			{
				m_auctionsIndexedByParentTypeExactMatch[auction->GetItem().GetCategory()].insert(std::make_pair(auction->GetItem().GetItemId(), auction));
			}

			if (auction->GetItem().GetItemTemplateId() != 0)
			{
				m_auctionsIndexedByTemplate[std::make_pair(auction->GetItem().GetCategory(), auction->GetItem().GetItemTemplateId())].insert(std::make_pair(auction->GetItem().GetItemId(), auction));
			}
		}
	}
	return true;
}

// ----------------------------------------------------------------------

//All vendor available items must be in the vendor offers list to be retrieved
void AuctionLocation::CancelVendorSale(Auction *auction)
{
	RemoveAuction(auction);
	m_vendorOffers.insert(std::make_pair(auction->GetItem().GetItemId(),
		auction));
}

// ----------------------------------------------------------------------

bool AuctionLocation::RemoveAuction(Auction *auction)
{
	assert(auction != nullptr);
	return RemoveAuction(auction->GetItem().GetItemId());
}

// ----------------------------------------------------------------------

bool AuctionLocation::RemoveAuction(const NetworkId & itemId)
{
	std::map<NetworkId, Auction *>::iterator i = m_auctions.find(itemId);
	if (i != m_auctions.end())
	{
		m_auctionItemCount -= (*i).second->GetItem().GetSize();
		if (m_auctionItemCount < 0)
		{
			m_auctionItemCount = 0;
		}

		if (m_auctionItemCount == 0)
		{
			// vendor has reached 0 item, so force an immediate update on the auction location
			SetNextUpdateTime(0);
		}

		if (AuctionItem::IsCategoryResourceContainer((*i).second->GetItem().GetCategory()))
		{
			m_auctionsResourceContainer.erase((*i).second->GetItem().GetItemId());
		}
		else if ((*i).second->GetItem().GetCategory() != 0)
		{
			m_auctionsIndexedByType[(*i).second->GetItem().GetCategory()].erase((*i).second->GetItem().GetItemId());

			if (GameObjectTypes::isSubType((*i).second->GetItem().GetCategory()))
			{
				m_auctionsIndexedByType[GameObjectTypes::getMaskedType((*i).second->GetItem().GetCategory())].erase((*i).second->GetItem().GetItemId());
			}
			else
			{
				m_auctionsIndexedByParentTypeExactMatch[(*i).second->GetItem().GetCategory()].erase((*i).second->GetItem().GetItemId());
			}

			if ((*i).second->GetItem().GetItemTemplateId() != 0)
			{
				m_auctionsIndexedByTemplate[std::make_pair((*i).second->GetItem().GetCategory(), (*i).second->GetItem().GetItemTemplateId())].erase((*i).second->GetItem().GetItemId());
			}
		}

		m_auctions.erase(i);
	}
	else
	{
		i = m_vendorOffers.find(itemId);
		if (i != m_vendorOffers.end())
		{
			m_vendorOffers.erase(i);
		}
	}
	return true;
}

// ----------------------------------------------------------------------

Auction *AuctionLocation::GetAuction(const NetworkId & itemId)
{
	std::map<NetworkId, Auction *>::iterator i = m_auctions.find(itemId);
	if (i != m_auctions.end())
	{
		return((*i).second);
	}
	else
	{
		i = m_vendorOffers.find(itemId);
		if (i != m_vendorOffers.end())
		{
			return((*i).second);
		}
	}
	return nullptr;
}

// ----------------------------------------------------------------------

int AuctionLocation::GetValue() const
{
	int value = 0;
	std::map<NetworkId, Auction *>::const_iterator i = m_auctions.begin();
	while (i != m_auctions.end())
	{
		Auction *auction = (*i).second;	
		if (auction->IsActive())
		{
			int auctionValue = auction->GetBuyNowPrice();
			if (auctionValue > 0)
			{
				value += auctionValue;
			}
		}
		++i;
	}
	return value;
}

// ----------------------------------------------------------------------

bool AuctionLocation::MatchLocation(const std::string & planet, const std::string & region, const NetworkId & locationId, bool checkLocationInfo, bool searchMyVendorsOnly, bool overrideVendorSearchFlag, const NetworkId & playerId, bool allowSearchVendors, const NetworkId & vendorId, const int searchType) const
{
	if (IsPacked())
	{
		return false;
	}

	if (!allowSearchVendors && IsVendorMarket())
	{
		return false;
	}

	if (searchMyVendorsOnly && playerId != m_ownerId)
	{
		return false;
	}

	if (!m_searchEnabled && !overrideVendorSearchFlag && !searchMyVendorsOnly && IsVendorMarket())
	{
		// Search is disabled on this vendor.  However search of this vendor should still be allowed
		// if player comes to the vendor to search.
		return (vendorId == m_locationId);
	}

	// do not show bazaar items while searching vendor items
	if (searchType == AST_ByVendorSelling && (! IsVendorMarket()))
		return false;

	if (checkLocationInfo)
	{
		if (locationId.isValid())
			return (locationId == m_locationId);

		if (!planet.empty() && (planet != m_locationPlanet))
			return false;

		if (!region.empty() && (region != m_locationRegion))
			return false;
	}

	return true;
}

// ----------------------------------------------------------------------

void AuctionLocation::SetOwnerId(const NetworkId & ownerId)
{
	AuctionMarket::getInstance().removeAuctionLocationFromLocationIndex(this);

	m_ownerId = ownerId;

	AuctionMarket::getInstance().addAuctionLocationToLocationIndex(this);
	SetNextUpdateTime(0); // force an immediate update on the auction location
}

// ----------------------------------------------------------------------

void AuctionLocation::SetLocationString( const std::string & newstr )
{
	AuctionMarket::getInstance().removeAuctionLocationFromLocationIndex(this);

	m_location = newstr;

	AuctionMarket::getPlanetAndRegionFromLocationString(m_location, m_locationPlanet, m_locationRegion);
	AuctionMarket::getInstance().addAuctionLocationToLocationIndex(this);
}

// ----------------------------------------------------------------------

void AuctionLocation::SetSearchedEnabled(bool enabled)
{
	AuctionMarket::getInstance().removeAuctionLocationFromLocationIndex(this);

	m_searchEnabled = enabled;

	AuctionMarket::getInstance().addAuctionLocationToLocationIndex(this);
}

// ----------------------------------------------------------------------

void AuctionLocation::SetSalesTax( int salesTax, const NetworkId & bankId )
{
	m_salesTax = salesTax;
	m_salesTaxBankId = bankId;

	//printf( "AuctionLocation::SetSalesTax() setting sales tax to %ld for bankId %Ld.\n", m_salesTax, m_salesTaxBankId.getValue());
}

// ----------------------------------------------------------------------

void AuctionLocation::SetNextUpdateTime(int nextUpdateTime)
{
	if (nextUpdateTime != m_nextUpdateTime)
	{
		AuctionMarket::getInstance().RemoveAuctionLocationFromPriorityQueue(*this);
	}

	m_nextUpdateTime = nextUpdateTime;
	AuctionMarket::getInstance().AddAuctionLocationToPriorityQueue(*this);
}

// ----------------------------------------------------------------------

void AuctionLocation::Update(int emptyDate, int lastAccessDate, int inactiveDate, int status)
{
	int oldStatus = GetStatus();
	if (emptyDate >= 0)
	{
		m_emptyDate = emptyDate;
		SetNextUpdateTime(0); // force an immediate update on the auction location
	}
	if (lastAccessDate >= 0)
	{
		m_lastAccessDate = lastAccessDate;
		m_vendorFirstTimerExpiredAuctionDate = 0;
		SetNextUpdateTime(0); // force an immediate update on the auction location
	}
	if (inactiveDate >= 0)
	{
		m_inactiveDate = inactiveDate;
		SetNextUpdateTime(0); // force an immediate update on the auction location
	}
	if (status >= AuctionMarket::ACTIVE && status <= AuctionMarket::REMOVED)
	{
		WARNING((status != nextStatus[GetStatus()][status]), ("Location status inconsistency, old status = %d, new status = %d, expected status = %d", GetStatus(), status, nextStatus[GetStatus()][status]));
		SetStatus(status);
	}

	CMUpdateLocationMessage msg(
		m_locationId, 
		m_ownerId,
		m_location, 
		m_salesTax, 
		m_salesTaxBankId, 
		m_emptyDate, 
		m_lastAccessDate, 
		m_inactiveDate, 
		m_status,
		m_searchEnabled,
		m_entranceCharge);
	
	DatabaseServerConnection* dbServer = CommodityServer::getInstance().getDatabaseServer();
	if (dbServer)	
		dbServer->send(msg, true);
	if (oldStatus == AuctionMarket::EMPTY && GetStatus() == AuctionMarket::ACTIVE)
		AuctionMarket::getInstance().OnVendorStatusChange(-1, m_locationId, ARC_VendorStatusNotEmpty);
}

// ----------------------------------------------------------------------

int AuctionLocation::GetStatus() const
{
	return m_status & cs_statusMask;
}

// ----------------------------------------------------------------------

int AuctionLocation::GetFullStatus() const
{
	return m_status;
}

// ----------------------------------------------------------------------

void AuctionLocation::SetStatus(int const status)
{
	AuctionMarket::getInstance().removeAuctionLocationFromLocationIndex(this);

	m_status = (m_status & ~cs_statusMask) | (status & cs_statusMask);

	AuctionMarket::getInstance().addAuctionLocationToLocationIndex(this);
	SetNextUpdateTime(0); // force an immediate update on the auction location
}

// ----------------------------------------------------------------------

void AuctionLocation::SetFullStatus(int const status, bool const upperBitsOnly)
{
	AuctionMarket::getInstance().removeAuctionLocationFromLocationIndex(this);

	if (upperBitsOnly)
		m_status = (m_status & cs_statusMask) | (status & ~cs_statusMask);
	else
		m_status = status;

	AuctionMarket::getInstance().addAuctionLocationToLocationIndex(this);
	SetNextUpdateTime(0); // force an immediate update on the auction location
}

// ----------------------------------------------------------------------

bool AuctionLocation::IsPacked() const
{
	return m_status & cs_packedFlag;
}

// ----------------------------------------------------------------------

std::map<NetworkId, Auction *> & AuctionLocation::GetAuctionsByType(int type)
{
	std::map<int, std::map<NetworkId, Auction *> >::iterator const iterFind = m_auctionsIndexedByType.find(type);
	if (iterFind != m_auctionsIndexedByType.end())
		return iterFind->second;

	return emptyAuctionList;
}

// ----------------------------------------------------------------------

std::map<NetworkId, Auction *> & AuctionLocation::GetAuctionsByParentTypeExactMatch(int type)
{
	std::map<int, std::map<NetworkId, Auction *> >::iterator const iterFind = m_auctionsIndexedByParentTypeExactMatch.find(type);
	if (iterFind != m_auctionsIndexedByParentTypeExactMatch.end())
		return iterFind->second;

	return emptyAuctionList;
}

// ----------------------------------------------------------------------

std::map<NetworkId, Auction *> & AuctionLocation::GetAuctionsByTemplate(int type, int templateId)
{
	std::map<std::pair<int, int>, std::map<NetworkId, Auction *> >::iterator const iterFind = m_auctionsIndexedByTemplate.find(std::make_pair(type, templateId));
	if (iterFind != m_auctionsIndexedByTemplate.end())
		return iterFind->second;

	return emptyAuctionList;
}

// ======================================================================
