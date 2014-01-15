#ifndef CleanupInvalidItemRetrievalMessage_h
#define CleanupInvalidItemRetrievalMessage_h

//#include "AuctionBase.h"
#include "sharedNetworkMessages/GameNetworkMessage.h"
#include "sharedFoundation/NetworkId.h"

class CleanupInvalidItemRetrievalMessage : public GameNetworkMessage
{

public:
	CleanupInvalidItemRetrievalMessage(int responseId, const NetworkId &  itemId);

	CleanupInvalidItemRetrievalMessage(Archive::ReadIterator & source);
	~CleanupInvalidItemRetrievalMessage();

	const NetworkId &          GetItemId() const {return m_itemId.get();}
	int                        GetResponseId () const {return m_responseId.get();}
	int                        GetTrackId    () const {return m_trackId.get();}
	void                       SetTrackId    (int trackId) {m_trackId.set(trackId);}

private:
	Archive::AutoVariable<int>       m_responseId;
	Archive::AutoVariable<int>       m_trackId;
	Archive::AutoVariable<NetworkId> m_itemId;

};

#endif
