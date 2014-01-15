#ifndef OnCancelAuctionMessage_h
#define OnCancelAuctionMessage_h

//#include "AuctionBase.h"
#include "sharedNetworkMessages/GameNetworkMessage.h"
#include "sharedFoundation/NetworkId.h"

class OnCancelAuctionMessage : public GameNetworkMessage
{

public:
	OnCancelAuctionMessage(
		int responseId,
		int resultCode,
		const NetworkId & itemId,
		const NetworkId & playerId,
		const NetworkId & highBidderId,
		int highBidAmount,
		const std::string & location
	);
	OnCancelAuctionMessage(Archive::ReadIterator & source);
	~OnCancelAuctionMessage();

	const NetworkId &          GetItemId()       const {return m_itemId.get();}
	const NetworkId &          GetPlayerId()     const {return m_playerId.get();}
	const NetworkId &          GetHighBidderId() const {return m_highBidderId.get();}
	int                        GetHighBid()      const {return m_highBidAmount.get();}
	const std::string &        GetLocation()     const {return m_location.get();}
	int                        GetResultCode()   const {return m_resultCode.get();}
	int                        GetResponseId () const {return m_responseId.get();}
	int                        GetTrackId    () const {return m_trackId.get();}
	void                       SetTrackId    (int trackId) {m_trackId.set(trackId);}

protected:

private:
	Archive::AutoVariable<int>              m_responseId;
	Archive::AutoVariable<int>              m_trackId;
	Archive::AutoVariable<NetworkId>        m_itemId;
	Archive::AutoVariable<NetworkId>        m_playerId;
	Archive::AutoVariable<NetworkId>        m_highBidderId;
	Archive::AutoVariable<int>              m_highBidAmount;
	Archive::AutoVariable<std::string>      m_location;
	Archive::AutoVariable<int>              m_resultCode;

};

#endif
