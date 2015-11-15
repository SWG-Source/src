#ifndef OnVendorRefuseItemMessage_h
#define OnVendorRefuseItemMessage_h

//#include "AuctionBase.h"
#include "sharedNetworkMessages/GameNetworkMessage.h"
#include "sharedFoundation/NetworkId.h"

class OnVendorRefuseItemMessage : public GameNetworkMessage
{

public:
	OnVendorRefuseItemMessage(
		int responseId,
		int resultCode,
		const NetworkId & itemId,
		const NetworkId & vendorId,
		const NetworkId & itemOwnerId
	);
	OnVendorRefuseItemMessage(Archive::ReadIterator & source);
	~OnVendorRefuseItemMessage();

	const NetworkId &          GetItemId()      const {return m_itemId.get();}
	const NetworkId &          GetVendorId()    const {return m_vendorId.get();}
	const NetworkId &          GetItemOwnerId()    const {return m_itemOwnerId.get();}
	int                        GetResultCode()  const {return m_resultCode.get();}
	int                        GetResponseId () const {return m_responseId.get();}
	int                        GetTrackId    () const {return m_trackId.get();}
	void                       SetTrackId    (int trackId) {m_trackId.set(trackId);}

protected:

private:
	Archive::AutoVariable<int>                    m_responseId;
	Archive::AutoVariable<int>                    m_trackId;
	Archive::AutoVariable<NetworkId>              m_itemId;
	Archive::AutoVariable<NetworkId>              m_vendorId;
	Archive::AutoVariable<NetworkId>              m_itemOwnerId;
	Archive::AutoVariable<int>                    m_resultCode;

};

#endif
