#ifndef SetEntranceChargeMessage_H
#define SetEntranceChargeMessage_H

#include "sharedNetworkMessages/GameNetworkMessage.h"
#include "sharedFoundation/NetworkId.h"
#include "Unicode.h"

class SetEntranceChargeMessage : public GameNetworkMessage
{

public:
	explicit SetEntranceChargeMessage(
		const int responseId,
		const int trackId,
		const NetworkId &vendorId,
		const int entranceCharge
	);

	SetEntranceChargeMessage(Archive::ReadIterator & source);
	~SetEntranceChargeMessage();

	const NetworkId &      GetVendorId()         const {return m_vendorId.get();}
	int		       GetEntranceCharge()   const {return m_entranceCharge.get();}
	int                    GetResponseId()       const {return m_responseId.get();}
	int                    GetTrackId()          const {return m_trackId.get();}
	void                   SetTrackId (int trackId)    {m_trackId.set(trackId);}

protected:

private:
	Archive::AutoVariable<int>          m_responseId;
	Archive::AutoVariable<int>          m_trackId;
	Archive::AutoVariable<NetworkId>    m_vendorId;
	Archive::AutoVariable<int>          m_entranceCharge;
};

#endif
