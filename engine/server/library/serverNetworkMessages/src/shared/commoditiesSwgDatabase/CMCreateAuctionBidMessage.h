#ifndef CMCREATEAUCTIONBIDMESSAGE_H
#define CMCREATEAUCTIONBIDMESSAGE_H

//#include "AuctionBase.h"
#include "sharedNetworkMessages/GameNetworkMessage.h"
#include "sharedFoundation/NetworkId.h"
#include "Unicode.h"

class CMCreateAuctionBidMessage : public GameNetworkMessage
{

public:
	explicit CMCreateAuctionBidMessage(
		const NetworkId &    itemId,
		const NetworkId &    bidderId,
		int                  bid,
		int                  maxProxyBid
	);

	CMCreateAuctionBidMessage(Archive::ReadIterator & source);
	~CMCreateAuctionBidMessage();

	const NetworkId &      GetItemId()        const {return m_itemId.get();}
	const NetworkId &      GetBidderId()      const {return m_bidderId.get();}
	int                    GetBid()           const {return m_bid.get();}
	int                    GetMaxProxyBid()   const {return m_maxProxyBid.get();}
		
protected:

private:
	Archive::AutoVariable<NetworkId>    m_itemId;
	Archive::AutoVariable<NetworkId>    m_bidderId;
	Archive::AutoVariable<int>          m_bid;
	Archive::AutoVariable<int>          m_maxProxyBid;
};

#endif
