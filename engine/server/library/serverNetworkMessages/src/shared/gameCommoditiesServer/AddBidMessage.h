#ifndef AddBidMessage_h
#define AddBidMessage_h

//#include "AuctionBase.h"
#include "sharedNetworkMessages/GameNetworkMessage.h"
#include "sharedFoundation/NetworkId.h"

class AddBidMessage : public GameNetworkMessage
{

public:
	AddBidMessage(
		int requestId,
		const NetworkId & auctionId,
		const NetworkId & playerId,
		int bid,
		int maxProxyBid,
		const std::string & playerName
	);
	AddBidMessage(Archive::ReadIterator & source);
	~AddBidMessage();

	const NetworkId &          GetAuctionId() const {return m_auctionId.get();}
	const NetworkId &          GetPlayerId() const {return m_playerId.get();}
	int                        GetBid() const {return m_bid.get();}
	int                        GetMaxProxyBid() const {return m_maxProxyBid.get();}
	const std::string &        GetPlayerName() const {return m_playerName.get();}
	int                        GetResponseId () const {return m_responseId.get();}
	int                        GetTrackId    () const {return m_trackId.get();}
	void                       SetTrackId    (int trackId) {m_trackId.set(trackId);}

protected:

private:
	Archive::AutoVariable<int>              m_responseId;
	Archive::AutoVariable<int>              m_trackId;
	Archive::AutoVariable<NetworkId>        m_auctionId;
	Archive::AutoVariable<NetworkId>        m_playerId;
	Archive::AutoVariable<int>              m_bid;
	Archive::AutoVariable<int>              m_maxProxyBid;
	Archive::AutoVariable<std::string>      m_playerName;

};

#endif
