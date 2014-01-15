#ifndef AcceptHighBidMessage_h
#define AcceptHighBidMessage_h

//#include "AuctionBase.h"
#include "sharedNetworkMessages/GameNetworkMessage.h"
#include "sharedFoundation/NetworkId.h"

class AcceptHighBidMessage : public GameNetworkMessage
{

public:
	AcceptHighBidMessage( int requestId, const NetworkId & auctionId, const NetworkId & playerId );
	explicit AcceptHighBidMessage(Archive::ReadIterator & source);
	~AcceptHighBidMessage();

	const NetworkId &      GetAuctionId() const {return m_auctionId.get();}
	const NetworkId &      GetPlayerId() const {return m_playerId.get();}
	int                    GetResponseId () const {return m_responseId.get();}
	int                    GetTrackId    () const {return m_trackId.get();}
	void                   SetTrackId    (int trackId) {m_trackId.set(trackId);}

protected:

private:
	Archive::AutoVariable<int>       m_responseId;
	Archive::AutoVariable<int>       m_trackId;
	Archive::AutoVariable<NetworkId> m_auctionId;
	Archive::AutoVariable<NetworkId> m_playerId;

};

#endif
