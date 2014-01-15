#ifndef GetAuctionLocationsMessage_H
#define GetAuctionLocationsMessage_H

#include "sharedNetworkMessages/GameNetworkMessage.h"
#include "sharedFoundation/NetworkId.h"
#include "Unicode.h"

class GetAuctionLocationsMessage : public GameNetworkMessage
{
public:
	struct AuctionLocation
	{
		NetworkId   locationId;
		int         locationNameLength;
		std::string locationName;
		NetworkId   ownerId;
		int         salesTax;
		NetworkId   salesTaxBankId;
		int         emptyDate;
		int         lastAccessDate;
		int         inactiveDate;
		int         status;
		bool		searchEnabled;
		int         entranceCharge;
	};

	GetAuctionLocationsMessage();
	GetAuctionLocationsMessage(Archive::ReadIterator & source);
	~GetAuctionLocationsMessage();

	void addAuctionLocation(
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
	);

	void clearAllAuctionLocations() {m_auctionLocations.get().clear();}

	const std::list<AuctionLocation> & getAuctionLocations() const {return m_auctionLocations.get();}

protected:

private:
	Archive::AutoList<AuctionLocation> m_auctionLocations;
};

namespace Archive
{
	inline void get(ReadIterator & source, GetAuctionLocationsMessage::AuctionLocation & target)
	{
		Archive::get(source, target.locationId);
		Archive::get(source, target.locationNameLength);
		Archive::get(source, target.locationName);
		Archive::get(source, target.ownerId);
		Archive::get(source, target.salesTax);
		Archive::get(source, target.salesTaxBankId);
		Archive::get(source, target.emptyDate);
		Archive::get(source, target.lastAccessDate);
		Archive::get(source, target.inactiveDate);
		Archive::get(source, target.status);
		Archive::get(source, target.searchEnabled);
		Archive::get(source, target.entranceCharge);
	}

	inline void put(ByteStream & target, const GetAuctionLocationsMessage::AuctionLocation & source)
	{
		Archive::put(target, source.locationId);
		Archive::put(target, source.locationNameLength);
		Archive::put(target, source.locationName);
		Archive::put(target, source.ownerId);
		Archive::put(target, source.salesTax);
		Archive::put(target, source.salesTaxBankId);
		Archive::put(target, source.emptyDate);
		Archive::put(target, source.lastAccessDate);
		Archive::put(target, source.inactiveDate);
		Archive::put(target, source.status);
		Archive::put(target, source.searchEnabled);
		Archive::put(target, source.entranceCharge);
	}
}

#endif
