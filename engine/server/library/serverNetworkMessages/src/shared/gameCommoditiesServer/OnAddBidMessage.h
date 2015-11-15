#ifndef OnAddBidMessage_h
#define OnAddBidMessage_h

//#include "AuctionBase.h"
#include "sharedNetworkMessages/GameNetworkMessage.h"
#include "sharedFoundation/NetworkId.h"
#include "Unicode.h"

class OnAddBidMessage : public GameNetworkMessage
{

public:
	OnAddBidMessage(
		int responseId,
		int resultCode,
		const NetworkId & itemId,
		const NetworkId & ownerId,
		const NetworkId & bidderId,
		const NetworkId & previousBidderId,
		int bidAmount,
		int previousBidAmount,
		int maxProxyBid,
		const std::string &location,
		int itemNameLength,
		const Unicode::String &itemName,
		int salesTaxAmount,
		const NetworkId & salesTaxBankId
	);
	OnAddBidMessage(Archive::ReadIterator & source);
	~OnAddBidMessage();

	const NetworkId &          GetItemId()      const {return m_itemId.get();}
	const NetworkId &          GetOwnerId()     const {return m_ownerId.get();}
	const NetworkId &          GetBidderId()    const {return m_bidderId.get();}
	const NetworkId &          GetPreviousBidderId()    const {return m_previousBidderId.get();}
	int                        GetBid()         const {return m_bidAmount.get();}
	int                        GetPreviousBid() const {return m_previousBidAmount.get();}
	int                        GetMaxProxyBid() const {return m_maxProxyBid.get();}
	const std::string &        GetLocation()    const {return m_location.get();}
	int                        GetResultCode()  const {return m_resultCode.get();}
	int                        GetItemNameLength()   const {return m_itemNameLength.get();}
	const Unicode::String &    GetItemName()   const {return m_itemName.get();}
	int                        GetSalesTaxAmount() const {return m_salesTaxAmount.get();}
	const NetworkId &          GetSalesTaxBankId() const {return m_salesTaxBankId.get();}
	int                        GetResponseId () const {return m_responseId.get();}
	int                        GetTrackId    () const {return m_trackId.get();}
	void                       SetTrackId    (int trackId) {m_trackId.set(trackId);}


protected:

private:
	Archive::AutoVariable<int>              m_responseId;
	Archive::AutoVariable<int>              m_trackId;
	Archive::AutoVariable<NetworkId>        m_itemId;
	Archive::AutoVariable<NetworkId>        m_ownerId;
	Archive::AutoVariable<NetworkId>        m_bidderId;
	Archive::AutoVariable<NetworkId>        m_previousBidderId;
	Archive::AutoVariable<int>              m_bidAmount;
	Archive::AutoVariable<int>              m_previousBidAmount;
	Archive::AutoVariable<int>              m_maxProxyBid;
	Archive::AutoVariable<std::string>      m_location;
	Archive::AutoVariable<int>              m_resultCode;
	Archive::AutoVariable<int>              m_itemNameLength;
	Archive::AutoVariable<Unicode::String>  m_itemName;
	Archive::AutoVariable<int>              m_salesTaxAmount;
	Archive::AutoVariable<NetworkId>        m_salesTaxBankId;
};

#endif
