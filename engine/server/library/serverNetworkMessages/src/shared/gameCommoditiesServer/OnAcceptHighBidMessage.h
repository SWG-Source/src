#ifndef OnAcceptHighBidMessage_h
#define OnAcceptHighBidMessage_h

//#include "AuctionBase.h"
#include "sharedNetworkMessages/GameNetworkMessage.h"
#include "sharedFoundation/NetworkId.h"

class OnAcceptHighBidMessage : public GameNetworkMessage
{

public:
	OnAcceptHighBidMessage(
		int responseId,
		int resultCode,
		const NetworkId &  itemId,
		const NetworkId &  playerId
	);
	OnAcceptHighBidMessage(Archive::ReadIterator & source);
	~OnAcceptHighBidMessage();

	const NetworkId &          GetItemId()   const {return m_itemId.get();}
	const NetworkId &          GetPlayerId() const {return m_playerId.get();}
	int                        GetResultCode() const {return m_resultCode.get();}
	int                        GetResponseId () const {return m_responseId.get();}
	int                        GetTrackId    () const {return m_trackId.get();}
	void                       SetTrackId    (int trackId) {m_trackId.set(trackId);}

protected:

private:
	Archive::AutoVariable<int>              m_responseId;
	Archive::AutoVariable<int>              m_trackId;
	Archive::AutoVariable<NetworkId>        m_itemId;
	Archive::AutoVariable<NetworkId>        m_playerId;
	Archive::AutoVariable<int>              m_resultCode;
};

#endif
