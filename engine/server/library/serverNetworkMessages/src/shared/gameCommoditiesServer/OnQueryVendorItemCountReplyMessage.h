#ifndef OnQueryVendorItemCountReplyMessage_H
#define OnQueryVendorItemCountReplyMessage_H

#include "sharedNetworkMessages/GameNetworkMessage.h"
#include "sharedFoundation/NetworkId.h"

class OnQueryVendorItemCountReplyMessage : public GameNetworkMessage
{

public:
	explicit OnQueryVendorItemCountReplyMessage(
		const int responseId,
		const NetworkId &vendorId,
		const int vendorItemCount,
		const bool searchEnabled
	);

	OnQueryVendorItemCountReplyMessage(Archive::ReadIterator & source);
	~OnQueryVendorItemCountReplyMessage();

	const int	       getResponseId()       const {return m_responseId.get();}
	const NetworkId &      getVendorId()         const {return m_vendorId.get();}
	const int	       getVendorItemCount()  const {return m_vendorItemCount.get();}
	const bool	       getSearchEnabled()    const {return m_searchEnabled.get();}

protected:

private:
	Archive::AutoVariable<int>          m_responseId;
	Archive::AutoVariable<NetworkId>    m_vendorId;
	Archive::AutoVariable<int>          m_vendorItemCount;
	Archive::AutoVariable<bool>         m_searchEnabled;
};

#endif
