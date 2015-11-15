#ifndef DestroyVendorMarketMessage_h
#define DestroyVendorMarketMessage_h

//#include "AuctionBase.h"
#include "sharedNetworkMessages/GameNetworkMessage.h"
#include "sharedFoundation/NetworkId.h"

class DestroyVendorMarketMessage : public GameNetworkMessage
{

public:
	DestroyVendorMarketMessage(
		int responseId,
		const NetworkId & ownerId,
		const std::string & ownerName,
		const std::string & location);


	DestroyVendorMarketMessage(Archive::ReadIterator & source);
	~DestroyVendorMarketMessage();

	const NetworkId &          GetOwnerId() const {return m_ownerId.get();}
	const std::string &        GetOwnerName() const {return m_ownerName.get();}
	const std::string &        GetLocation() const {return m_location.get();}
	int                        GetResponseId () const {return m_responseId.get();}
	int                        GetTrackId    () const {return m_trackId.get();}
	void                       SetTrackId    (int trackId) {m_trackId.set(trackId);}

protected:

private:
	Archive::AutoVariable<int>              m_responseId;
	Archive::AutoVariable<int>              m_trackId;
	Archive::AutoVariable<NetworkId>        m_ownerId;
	Archive::AutoVariable<std::string>      m_ownerName;
	Archive::AutoVariable<std::string>      m_location;

};

#endif
