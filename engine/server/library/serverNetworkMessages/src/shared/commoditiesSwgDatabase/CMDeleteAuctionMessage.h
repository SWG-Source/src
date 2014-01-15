#ifndef CMDELETEAUCTIONMESSAGE_H
#define CMDELETEAUCTIONMESSAGE_H

//#include "AuctionBase.h"
#include "sharedNetworkMessages/GameNetworkMessage.h"
#include "sharedFoundation/NetworkId.h"
#include "Unicode.h"

class CMDeleteAuctionMessage : public GameNetworkMessage
{

public:
	explicit CMDeleteAuctionMessage(
		const NetworkId &    itemId
	);

	CMDeleteAuctionMessage(Archive::ReadIterator & source);
	~CMDeleteAuctionMessage();

	const NetworkId &      GetItemId()        const {return m_itemId.get();}
		
protected:

private:
	Archive::AutoVariable<NetworkId>    m_itemId;
};

#endif
