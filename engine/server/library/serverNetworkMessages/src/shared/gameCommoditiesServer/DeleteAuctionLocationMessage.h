#ifndef DeleteAuctionLocationMessage_h
#define DeleteAuctionLocationMessage_h

//#include "AuctionBase.h"
#include "sharedNetworkMessages/GameNetworkMessage.h"
#include "sharedFoundation/NetworkId.h"

class DeleteAuctionLocationMessage : public GameNetworkMessage
{

public:
	DeleteAuctionLocationMessage(
		int responseId,
		const NetworkId & locationId,
		const std::string & whoRequested);


	DeleteAuctionLocationMessage(Archive::ReadIterator & source);
	~DeleteAuctionLocationMessage();

	const NetworkId &          GetLocationId()       const {return m_locationId.get();}
	const std::string &        GetWhoRequested()     const {return m_whoRequested.get();}
	int                        GetResponseId ()      const {return m_responseId.get();}
	int                        GetTrackId    ()      const {return m_trackId.get();}
	void                       SetTrackId    (int trackId) {m_trackId.set(trackId);}

protected:

private:
	Archive::AutoVariable<int>              m_responseId;
	Archive::AutoVariable<int>              m_trackId;
	Archive::AutoVariable<NetworkId>        m_locationId;
	Archive::AutoVariable<std::string>      m_whoRequested;

};

#endif
