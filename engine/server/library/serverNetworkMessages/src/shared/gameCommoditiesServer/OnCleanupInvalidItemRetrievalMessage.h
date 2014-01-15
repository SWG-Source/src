#ifndef OnCleanupInvalidItemRetrievalMessage_h
#define OnCleanupInvalidItemRetrievalMessage_h

//#include "AuctionBase.h"
#include "sharedNetworkMessages/GameNetworkMessage.h"
#include "sharedFoundation/NetworkId.h"

class OnCleanupInvalidItemRetrievalMessage : public GameNetworkMessage
{

public:
	OnCleanupInvalidItemRetrievalMessage(
		int responseId,
		const NetworkId & itemId,
		const NetworkId & playerId,
		const NetworkId & creatorId,
		int reimburseAmt
	);
	OnCleanupInvalidItemRetrievalMessage(Archive::ReadIterator & source);
	~OnCleanupInvalidItemRetrievalMessage();

	const NetworkId &          GetItemId()      const {return m_itemId.get();}
	const NetworkId &          GetPlayerId()    const {return m_playerId.get();}
	const NetworkId &          GetCreatorId()    const {return m_creatorId.get();}
	int                        GetReimburseAmt()  const {return m_reimburseAmt.get();}
	int                        GetResponseId () const {return m_responseId.get();}
	int                        GetTrackId    () const {return m_trackId.get();}
	void                       SetTrackId    (int trackId) {m_trackId.set(trackId);}

protected:

private:
	Archive::AutoVariable<int>              m_responseId;
	Archive::AutoVariable<int>              m_trackId;
	Archive::AutoVariable<NetworkId>        m_itemId;
	Archive::AutoVariable<NetworkId>        m_playerId;
	Archive::AutoVariable<NetworkId>        m_creatorId;
	Archive::AutoVariable<int>              m_reimburseAmt;
};

#endif
