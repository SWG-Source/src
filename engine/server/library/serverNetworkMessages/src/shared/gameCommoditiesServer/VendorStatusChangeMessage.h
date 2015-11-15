#ifndef VendorStatusChangeMessage_H
#define VendorStatusChangeMessage_H

#include "sharedNetworkMessages/GameNetworkMessage.h"
#include "sharedFoundation/NetworkId.h"
#include "serverNetworkMessages/AuctionBase.h"

class VendorStatusChangeMessage : public GameNetworkMessage
{

public:
	explicit VendorStatusChangeMessage(
		const NetworkId &vendorId,
		const int status
	);

	VendorStatusChangeMessage(Archive::ReadIterator & source);
	~VendorStatusChangeMessage();

	const NetworkId &      getVendorId()         const {return m_vendorId.get();}
	const int              getStatus()           const {return m_status.get();}

protected:

private:
	Archive::AutoVariable<NetworkId>    m_vendorId;
	Archive::AutoVariable<int>          m_status;
};

#endif
