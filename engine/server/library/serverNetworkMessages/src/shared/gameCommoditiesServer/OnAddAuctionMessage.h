#ifndef OnAddAuctionMessage_h
#define OnAddAuctionMessage_h

//#include "AuctionBase.h"
#include "sharedNetworkMessages/GameNetworkMessage.h"
#include "sharedFoundation/NetworkId.h"

class OnAddAuctionMessage : public GameNetworkMessage
{
public:
	OnAddAuctionMessage(
		int responseId,
		int resultCode,
		const NetworkId & itemId,
		const NetworkId & ownerId,
		const std::string & ownerName,
		const NetworkId & vendorId,
		const std::string & location);

	OnAddAuctionMessage(Archive::ReadIterator & source);
	~OnAddAuctionMessage();

	const NetworkId &          GetOwnerId() const {return m_ownerId.get();}
	const std::string &        GetOwnerName() const {return m_ownerName.get();}
	const NetworkId &          GetItemId() const {return m_itemId.get();}
	const NetworkId &          GetVendorId() const {return m_vendorId.get();}
	int                        GetResultCode() const {return m_resultCode.get();}
	const std::string &        GetLocation() const {return m_location.get();}
	int                        GetResponseId () const {return m_responseId.get();}
	int                        GetTrackId    () const {return m_trackId.get();}
	void                       SetTrackId    (int trackId) {m_trackId.set(trackId);}

protected:

private:
	Archive::AutoVariable<int>              m_responseId;
	Archive::AutoVariable<int>              m_trackId;
	Archive::AutoVariable<NetworkId>        m_itemId;
	Archive::AutoVariable<NetworkId>        m_ownerId;
	Archive::AutoVariable<int>              m_resultCode;
	Archive::AutoVariable<NetworkId>        m_vendorId;
	Archive::AutoVariable<std::string>      m_ownerName;
	Archive::AutoVariable<std::string>      m_location;
};

#endif
