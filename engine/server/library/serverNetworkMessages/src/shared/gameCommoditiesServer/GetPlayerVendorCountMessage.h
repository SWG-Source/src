#ifndef GetPlayerVendorCountMessage_h
#define GetPlayerVendorCountMessage_h

#include "sharedNetworkMessages/GameNetworkMessage.h"

class GetPlayerVendorCountMessage : public GameNetworkMessage
{
public:
	GetPlayerVendorCountMessage(
		int responseId,
		const NetworkId & playerId);


	GetPlayerVendorCountMessage(Archive::ReadIterator & source);
	~GetPlayerVendorCountMessage();

	const NetworkId &          GetPlayerId() const {return m_playerId.get();}
	int                        GetResponseId () const {return m_responseId.get();}
	int                        GetTrackId    () const {return m_trackId.get();}
	void                       SetTrackId    (int trackId) {m_trackId.set(trackId);}

protected:

private:
	Archive::AutoVariable<int>              m_responseId;
	Archive::AutoVariable<int>              m_trackId;
	Archive::AutoVariable<NetworkId>        m_playerId;
};

#endif
