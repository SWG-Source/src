#include "serverNetworkMessages/FirstServerNetworkMessages.h"
#include "GetAuctionLocationsMessage.h"

GetAuctionLocationsMessage::GetAuctionLocationsMessage(Archive::ReadIterator & source) :
	GameNetworkMessage("GetAuctionLocationsMessage"),
	m_auctionLocations()
{
	AutoByteStream::addVariable(m_auctionLocations);
	unpack(source);
}

GetAuctionLocationsMessage::GetAuctionLocationsMessage() :
	GameNetworkMessage("GetAuctionLocationsMessage"),
	m_auctionLocations()
{
	AutoByteStream::addVariable(m_auctionLocations);
}

GetAuctionLocationsMessage::~GetAuctionLocationsMessage()
{
}

void GetAuctionLocationsMessage::addAuctionLocation(
	const NetworkId &   locationId,
	int                 locationNameLength,
	const std::string & locationName,
	const NetworkId &   ownerId,
	int                 salesTax,
	const NetworkId &   salesTaxBankId,
	int                 emptyDate,
	int                 lastAccessDate,
	int                 inactiveDate,
	int                 status,
	bool		        searchEnabled,
	int                 entranceCharge
)
{
	GetAuctionLocationsMessage::AuctionLocation auctionLocation;
	auctionLocation.locationId         = locationId;
	auctionLocation.locationNameLength = locationNameLength;
	auctionLocation.locationName       = locationName;
	auctionLocation.ownerId            = ownerId;
	auctionLocation.salesTax           = salesTax;
	auctionLocation.salesTaxBankId     = salesTaxBankId;
	auctionLocation.emptyDate          = emptyDate;
	auctionLocation.lastAccessDate     = lastAccessDate;
	auctionLocation.inactiveDate       = inactiveDate;
	auctionLocation.status             = status;
	auctionLocation.searchEnabled      = searchEnabled;
	auctionLocation.entranceCharge     = entranceCharge;

	m_auctionLocations.get().push_back(auctionLocation);
}
