#ifndef CreateVendorMarketMessage_h
#define CreateVendorMarketMessage_h

//#include "AuctionBase.h"
#include "sharedNetworkMessages/GameNetworkMessage.h"
#include "sharedFoundation/NetworkId.h"

class CreateVendorMarketMessage : public GameNetworkMessage
{

public:
	CreateVendorMarketMessage(
		int                 responseId,
		const NetworkId &   ownerId,
		const std::string & location,
		int                 playerVendorLimit,
		int                 entranceCharge);

	CreateVendorMarketMessage(Archive::ReadIterator & source);
	~CreateVendorMarketMessage();

	const NetworkId &          GetOwnerId()     const {return m_ownerId.get();}
	const std::string &        GetLocation()    const {return m_location.get();}
	int                        GetVendorLimit() const {return m_playerVendorLimit.get();}
	int                        GetEntranceCharge() const {return m_entranceCharge.get();}
	int                        GetResponseId () const {return m_responseId.get();}
	int                        GetTrackId    () const {return m_trackId.get();}
	void                       SetTrackId       (int trackId) {m_trackId.set(trackId);}

protected:

private:
	Archive::AutoVariable<int>              m_responseId;
	Archive::AutoVariable<int>              m_trackId;
	Archive::AutoVariable<NetworkId>        m_ownerId;
	Archive::AutoVariable<std::string>      m_location;
	Archive::AutoVariable<int>              m_playerVendorLimit;
	Archive::AutoVariable<int>              m_entranceCharge;
};

#endif
