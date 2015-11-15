#ifndef GetVendorOwnerMessage_h
#define GetVendorOwnerMessage_h

//#include "AuctionBase.h"
#include "sharedNetworkMessages/GameNetworkMessage.h"
#include "sharedFoundation/NetworkId.h"

class GetVendorOwnerMessage : public GameNetworkMessage
{

public:
	GetVendorOwnerMessage(
		int responseId,
		const NetworkId & ownerId,
		const std::string & location);


	GetVendorOwnerMessage(Archive::ReadIterator & source);
	~GetVendorOwnerMessage();

	const NetworkId &          GetOwnerId() const {return m_ownerId.get();}
	const std::string &        GetLocation() const {return m_location.get();}
	int                        GetResponseId () const {return m_responseId.get();}
	int                        GetTrackId    () const {return m_trackId.get();}
	void                       SetTrackId    (int trackId) {m_trackId.set(trackId);}

protected:

private:
	Archive::AutoVariable<int>              m_responseId;
	Archive::AutoVariable<int>              m_trackId;
	Archive::AutoVariable<NetworkId>        m_ownerId;
	Archive::AutoVariable<std::string>      m_location;
};

#endif
