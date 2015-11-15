#ifndef OnCreateVendorMarketMessage_h
#define OnCreateVendorMarketMessage_h

//#include "AuctionBase.h"
#include "sharedNetworkMessages/GameNetworkMessage.h"
#include "sharedFoundation/NetworkId.h"

class OnCreateVendorMarketMessage : public GameNetworkMessage
{

public:
	OnCreateVendorMarketMessage(int responseId, int resultCode, const NetworkId & ownerId, const std::string & location );
	OnCreateVendorMarketMessage(Archive::ReadIterator & source);
	~OnCreateVendorMarketMessage();

	const NetworkId &          GetOwnerId() const {return m_ownerId.get();}
	const std::string &        GetLocation() const {return m_location.get();}
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
	Archive::AutoVariable<int>              m_resultCode;

};

#endif
