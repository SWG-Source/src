#ifndef OnUpdateVendorSearchOptionMessage_H
#define OnUpdateVendorSearchOptionMessage_H

#include "sharedNetworkMessages/GameNetworkMessage.h"
#include "sharedFoundation/NetworkId.h"
#include "Unicode.h"

class OnUpdateVendorSearchOptionMessage : public GameNetworkMessage
{

public:
	explicit OnUpdateVendorSearchOptionMessage(
		const NetworkId & playerId,
		const bool enabled
	);

	OnUpdateVendorSearchOptionMessage(Archive::ReadIterator & source);
	~OnUpdateVendorSearchOptionMessage();

	bool		       GetEnabled()	     const {return m_enabled.get();}
	const NetworkId &      GetPlayerId()         const {return m_playerId.get();}

protected:

private:
	Archive::AutoVariable<NetworkId>    m_playerId;
	Archive::AutoVariable<bool>         m_enabled;
};

#endif
