#ifndef CMDELETELOCATIONMESSAGE_H
#define CMDELETELOCATIONMESSAGE_H

//#include "AuctionBase.h"
#include "sharedNetworkMessages/GameNetworkMessage.h"
#include "sharedFoundation/NetworkId.h"
#include "Unicode.h"

class CMDeleteLocationMessage : public GameNetworkMessage
{

public:
	explicit CMDeleteLocationMessage(
		const NetworkId &    locationId
	);

	CMDeleteLocationMessage(Archive::ReadIterator & source);
	~CMDeleteLocationMessage();

	const NetworkId &      GetLocationId()        const {return m_locationId.get();}
		
protected:

private:
	Archive::AutoVariable<NetworkId>    m_locationId;
};

#endif
