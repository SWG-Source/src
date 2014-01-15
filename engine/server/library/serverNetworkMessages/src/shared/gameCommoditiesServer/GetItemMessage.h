#ifndef GetItemMessage_h
#define GetItemMessage_h

//#include "AuctionBase.h"
#include "sharedNetworkMessages/GameNetworkMessage.h"
#include "sharedFoundation/NetworkId.h"

class GetItemMessage : public GameNetworkMessage
{

public:
	GetItemMessage(
		int requestId,
		const NetworkId &  itemId,
		const NetworkId &  playerId,
		const std::string & location
	);
	GetItemMessage(Archive::ReadIterator & source);
	~GetItemMessage();

	const NetworkId &          GetItemId()   const {return m_itemId.get();}
	const NetworkId &          GetPlayerId() const {return m_playerId.get();}
	const std::string &        GetLocation() const {return m_location.get();}
	int                        GetResponseId () const {return m_responseId.get();}
	int                        GetTrackId    () const {return m_trackId.get();}
	void                       SetTrackId    (int trackId) {m_trackId.set(trackId);}

protected:

private:
	Archive::AutoVariable<int>              m_responseId;
	Archive::AutoVariable<int>              m_trackId;
	Archive::AutoVariable<NetworkId>        m_itemId;
	Archive::AutoVariable<NetworkId>        m_playerId;
	Archive::AutoVariable<std::string>      m_location;

};

#endif
