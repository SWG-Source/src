#ifndef CMUPDATEAUCTIONMESSAGE_H
#define CMUPDATEAUCTIONMESSAGE_H

//#include "AuctionBase.h"
#include "sharedNetworkMessages/GameNetworkMessage.h"
#include "sharedFoundation/NetworkId.h"
#include "Unicode.h"

class CMUpdateAuctionMessage : public GameNetworkMessage
{

public:
	explicit CMUpdateAuctionMessage(
		const NetworkId &    itemId,
		const NetworkId &    ownerId,
		int                  flags
	);

	CMUpdateAuctionMessage(Archive::ReadIterator & source);
	~CMUpdateAuctionMessage();

	const NetworkId &      GetItemId()        const {return m_itemId.get();}
	const NetworkId &      GetOwnerId()         const {return m_ownerId.get();}
	int                    GetFlags()         const {return m_flags.get();}
		
protected:

private:
	Archive::AutoVariable<NetworkId>    m_itemId;
	Archive::AutoVariable<NetworkId>    m_ownerId;
	Archive::AutoVariable<int>          m_flags;
};

#endif
