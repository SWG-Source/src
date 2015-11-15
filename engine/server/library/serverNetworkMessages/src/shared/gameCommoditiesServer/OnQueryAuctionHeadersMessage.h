#ifndef OnQueryAuctionHeadersMessage_h
#define OnQueryAuctionHeadersMessage_h

#include "AuctionData.h"
#include "sharedNetworkMessages/GameNetworkMessage.h"
#include "sharedFoundation/NetworkId.h"
#include "sharedFoundation/NetworkIdArchive.h"
#include "unicodeArchive/UnicodeArchive.h"

#include <string>
#include <vector>

// ----------------------------------------------------------------------

class OnQueryAuctionHeadersMessage : public GameNetworkMessage
{

public:
	OnQueryAuctionHeadersMessage(
		int responseId,
		int resultCode,
		const NetworkId & playerId,
		int queryType,
		const std::vector<AuctionDataHeader> & auctions,
		unsigned int queryOffset,
		bool hasMorePages
	);
	OnQueryAuctionHeadersMessage(Archive::ReadIterator & source);
	~OnQueryAuctionHeadersMessage();

	typedef std::vector<AuctionDataHeader> ADV;

	const NetworkId &        GetPlayerId()    const {return m_playerId.get();}
	int                      GetQueryType()   const {return m_queryType.get();}
	int                      GetNumAuctions() const {return m_numAuctions.get();}
	const std::vector<AuctionDataHeader> &        GetAuctionData() const {return m_auctionData.get();}
	int                      GetResultCode()  const {return m_resultCode.get();}
	unsigned int             GetQueryOffset()  const {return m_queryOffset.get();}
	bool                     HasMorePages()  const {return m_hasMorePages.get();}
	int                      GetResponseId () const {return m_responseId.get();}
	int                      GetTrackId    () const {return m_trackId.get();}
	void                     SetTrackId    (int trackId) {m_trackId.set(trackId);}

protected:

private:
	Archive::AutoVariable<int>                    m_responseId;
	Archive::AutoVariable<int>                    m_trackId;
	Archive::AutoVariable<NetworkId>              m_playerId;
	Archive::AutoVariable<int>                    m_queryType;
	Archive::AutoVariable<int>                    m_numAuctions;
	Archive::AutoVariable<ADV>                    m_auctionData;
	Archive::AutoVariable<int>                    m_resultCode;
	Archive::AutoVariable<unsigned int>           m_queryOffset;
	Archive::AutoVariable<bool>                   m_hasMorePages;

};

// ----------------------------------------------------------------------

namespace Archive
{

	inline void get(ReadIterator &source, AuctionDataHeader &target)
	{
		get(source, target.type);
		get(source, target.auctionId);
		get(source, target.itemId);
		get(source, target.itemNameLength);
		get(source, target.itemName);
		get(source, target.minBid);
		get(source, target.highBid);
		get(source, target.timer);
		get(source, target.buyNowPrice);
		get(source, target.location);
		get(source, target.ownerId);
		get(source, target.highBidderId);
		get(source, target.maxProxyBid);
		get(source, target.myBid);
		get(source, target.itemType);
		get(source, target.resourceContainerClassCrc);
		get(source, target.flags);
		get(source, target.entranceCharge);
	}

// ----------------------------------------------------------------------

	inline void put(ByteStream &target, AuctionDataHeader const &source)
	{
		put(target, source.type);
		put(target, source.auctionId);
		put(target, source.itemId);
		put(target, source.itemNameLength);
		put(target, source.itemName);
		put(target, source.minBid);
		put(target, source.highBid);
		put(target, source.timer);
		put(target, source.buyNowPrice);
		put(target, source.location);
		put(target, source.ownerId);
		put(target, source.highBidderId);
		put(target, source.maxProxyBid);
		put(target, source.myBid);
		put(target, source.itemType);
		put(target, source.resourceContainerClassCrc);
		put(target, source.flags);
		put(target, source.entranceCharge);
	}

}

// ----------------------------------------------------------------------

#endif
