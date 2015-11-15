#ifndef LoadCommoditiesMessage_H
#define LoadCommoditiesMessage_H

#include "sharedNetworkMessages/GameNetworkMessage.h"
#include "sharedFoundation/NetworkId.h"
#include "Unicode.h"

class LoadCommoditiesMessage : public GameNetworkMessage
{

public:
	explicit LoadCommoditiesMessage(
		const int            payload
	);

	LoadCommoditiesMessage(Archive::ReadIterator & source);
	~LoadCommoditiesMessage();

	const int              getPayload() const {return m_payload.get();}
		
protected:

private:
	Archive::AutoVariable<int> m_payload;
};

#endif
