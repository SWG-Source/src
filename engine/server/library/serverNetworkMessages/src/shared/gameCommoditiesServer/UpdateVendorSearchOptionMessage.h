#ifndef UpdateVendorSearchOptionMessage_H
#define UpdateVendorSearchOptionMessage_H

#include "sharedNetworkMessages/GameNetworkMessage.h"
#include "sharedFoundation/NetworkId.h"
#include "Unicode.h"

class UpdateVendorSearchOptionMessage : public GameNetworkMessage
{

public:
	explicit UpdateVendorSearchOptionMessage(
		const int responseId,
		const int trackId,
		const NetworkId &vendorId,
		const bool enabled,
		const NetworkId & playerId
	);

	UpdateVendorSearchOptionMessage(Archive::ReadIterator & source);
	~UpdateVendorSearchOptionMessage();

	const NetworkId &      GetVendorId()         const {return m_vendorId.get();}
	bool		       GetEnabled()	     const {return m_enabled.get();}
	int                    GetResponseId()       const {return m_responseId.get();}
	const NetworkId &      GetPlayerId()         const {return m_playerId.get();}
	int                    GetTrackId()          const {return m_trackId.get();}
	void                   SetTrackId (int trackId)    {m_trackId.set(trackId);}

protected:

private:
	Archive::AutoVariable<int>          m_responseId;
	Archive::AutoVariable<int>          m_trackId;
	Archive::AutoVariable<NetworkId>    m_vendorId;
	Archive::AutoVariable<bool>         m_enabled;
	Archive::AutoVariable<NetworkId>    m_playerId;
};

#endif
