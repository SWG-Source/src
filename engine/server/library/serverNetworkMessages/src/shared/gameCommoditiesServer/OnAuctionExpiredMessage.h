#ifndef OnAuctionExpiredMessage_h
#define OnAuctionExpiredMessage_h

//#include "AuctionBase.h"
#include "sharedNetworkMessages/GameNetworkMessage.h"
#include "sharedFoundation/NetworkId.h"
#include "Unicode.h"

class OnAuctionExpiredMessage : public GameNetworkMessage
{

public:
	OnAuctionExpiredMessage(
		const NetworkId & ownerId,
		bool  sold,
		const NetworkId & buyerId,
		int bidAmount,
		const NetworkId & itemId,
		int maxProxyBid,
		const std::string & location,
		bool immediate,
		int itemNameLength,
		const Unicode::String &itemName,
		bool sendSellerMail
	);
	OnAuctionExpiredMessage(Archive::ReadIterator & source);
	~OnAuctionExpiredMessage();

	const NetworkId &          GetItemId()         const {return m_itemId.get();}
	const NetworkId &          GetOwnerId()        const {return m_ownerId.get();}
	const NetworkId &          GetBuyerId()        const {return m_buyerId.get();}
	int                        GetBid()            const {return m_bidAmount.get();}
	int                        GetMaxProxyBid()    const {return m_maxProxyBid.get();}
	const std::string &        GetLocation()       const {return m_location.get();}
	bool                       IsSold()            const {return m_sold.get();}
	bool                       IsImmediate()       const {return m_immediate.get();}
	int                        GetItemNameLength() const {return m_itemNameLength.get();}
	const Unicode::String &    GetItemName()       const {return m_itemName.get();}
	int                        GetResponseId () const {return m_responseId.get();}
	int                        GetTrackId    () const {return m_trackId.get();}
	void                       SetTrackId    (int trackId) {m_trackId.set(trackId);}
	bool                       GetSendSellerMail() const {return m_sendSellerMail.get();}

protected:

private:
	Archive::AutoVariable<int>              m_responseId;
	Archive::AutoVariable<int>              m_trackId;
	Archive::AutoVariable<NetworkId>        m_ownerId;
	Archive::AutoVariable<bool>             m_sold;
	Archive::AutoVariable<NetworkId>        m_buyerId;
	Archive::AutoVariable<int>              m_bidAmount;
	Archive::AutoVariable<NetworkId>        m_itemId;
	Archive::AutoVariable<int>              m_maxProxyBid;
	Archive::AutoVariable<std::string>      m_location;
	Archive::AutoVariable<bool>             m_immediate;
	Archive::AutoVariable<int>              m_itemNameLength;
	Archive::AutoVariable<Unicode::String>  m_itemName;
	Archive::AutoVariable<bool>             m_sendSellerMail;
};

#endif
