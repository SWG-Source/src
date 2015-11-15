#ifndef OnPermanentAuctionPurchasedMessage_h
#define OnPermanentAuctionPurchasedMessage_h

//#include "AuctionBase.h"
#include "sharedNetworkMessages/GameNetworkMessage.h"
#include "sharedFoundation/NetworkId.h"

class OnPermanentAuctionPurchasedMessage : public GameNetworkMessage
{

public:
	OnPermanentAuctionPurchasedMessage(
		const NetworkId & ownerId,
		const NetworkId & buyerId,
		int price,
		const NetworkId & itemId,
		const std::string & location,
		int itemNameLength,
		const Unicode::String & itemName,
		std::vector<std::pair<std::string, Unicode::String> > const & attributes
	);
	OnPermanentAuctionPurchasedMessage(Archive::ReadIterator & source);
	~OnPermanentAuctionPurchasedMessage();

	const NetworkId &          GetItemId()         const {return m_itemId.get();}
	const NetworkId &          GetOwnerId()        const {return m_ownerId.get();}
	const NetworkId &          GetBuyerId()        const {return m_buyerId.get();}
	int                        GetPrice()          const {return m_price.get();}
	const std::string &        GetLocation()       const {return m_location.get();}
	int                        GetItemNameLength() const {return m_itemNameLength.get();}
	const Unicode::String &    GetItemName()       const {return m_itemName.get();}
	std::vector<std::pair<std::string, Unicode::String> > const & GetAttributes() {return m_attributes.get();}
	int                        GetResponseId () const {return m_responseId.get();}
	int                        GetTrackId    () const {return m_trackId.get();}
	void                       SetTrackId    (int trackId) {m_trackId.set(trackId);}

protected:

private:
	Archive::AutoVariable<int>              m_responseId;
	Archive::AutoVariable<int>              m_trackId;
	Archive::AutoVariable<NetworkId>        m_ownerId;
	Archive::AutoVariable<NetworkId>        m_buyerId;
	Archive::AutoVariable<int>              m_price;
	Archive::AutoVariable<NetworkId>        m_itemId;
	Archive::AutoVariable<std::string>      m_location;
	Archive::AutoVariable<int>              m_itemNameLength;
	Archive::AutoVariable<Unicode::String>  m_itemName;
	Archive::AutoArray<std::pair<std::string, Unicode::String> > m_attributes;

};

#endif
