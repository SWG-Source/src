#ifndef QueryVendorItemCountMessage_H
#define QueryVendorItemCountMessage_H

#include "sharedNetworkMessages/GameNetworkMessage.h"
#include "sharedFoundation/NetworkId.h"
#include "Unicode.h"

class QueryVendorItemCountMessage : public GameNetworkMessage
{

public:
	explicit QueryVendorItemCountMessage(
		const int responseId,
		const int trackId,
		const NetworkId &vendorId
	);

	QueryVendorItemCountMessage(Archive::ReadIterator & source);
	~QueryVendorItemCountMessage();

	const NetworkId &      GetVendorId()         const {return m_vendorId.get();}
	int                    GetResponseId()       const {return m_responseId.get();}
	int                    GetTrackId()          const {return m_trackId.get();}
	void                   SetTrackId (int trackId)    {m_trackId.set(trackId);}

protected:

private:
	Archive::AutoVariable<int>          m_responseId;
	Archive::AutoVariable<int>          m_trackId;
	Archive::AutoVariable<NetworkId>    m_vendorId;
};

#endif
