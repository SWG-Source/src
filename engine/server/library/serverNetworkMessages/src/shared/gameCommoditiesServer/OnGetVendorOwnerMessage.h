#ifndef OnGetVendorOwnerMessage_h
#define OnGetVendorOwnerMessage_h

//#include "AuctionBase.h"
#include "sharedNetworkMessages/GameNetworkMessage.h"
#include "sharedFoundation/NetworkId.h"

class OnGetVendorOwnerMessage : public GameNetworkMessage
{

public:
	OnGetVendorOwnerMessage(
		int responseId,
		int resultCode,
		const NetworkId & ownerId,
		const NetworkId & requesterId,
		const std::string & location
);
	OnGetVendorOwnerMessage(Archive::ReadIterator & source);
	~OnGetVendorOwnerMessage();

	const NetworkId &          GetOwnerId() const {return m_ownerId.get();}
	const std::string &        GetLocation() const {return m_location.get();}
	const NetworkId &          GetRequesterId() const {return m_requesterId.get();}
	int                        GetResultCode() const {return m_resultCode.get();}
	int                        GetResponseId () const {return m_responseId.get();}
	int                        GetTrackId    () const {return m_trackId.get();}
	void                       SetTrackId    (int trackId) {m_trackId.set(trackId);}

protected:

private:
	Archive::AutoVariable<int>              m_responseId;
	Archive::AutoVariable<int>              m_trackId;
	Archive::AutoVariable<NetworkId>        m_ownerId;
	Archive::AutoVariable<std::string>      m_location;
	Archive::AutoVariable<NetworkId>        m_requesterId;
	Archive::AutoVariable<int>              m_resultCode;

};

#endif
