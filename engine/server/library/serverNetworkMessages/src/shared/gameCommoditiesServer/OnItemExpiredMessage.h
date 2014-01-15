#ifndef OnItemExpiredMessage_h
#define OnItemExpiredMessage_h

//#include "AuctionBase.h"
#include "sharedNetworkMessages/GameNetworkMessage.h"
#include "sharedFoundation/NetworkId.h"

class OnItemExpiredMessage : public GameNetworkMessage
{

public:
	OnItemExpiredMessage(
		const NetworkId & ownerId,
		const NetworkId & itemId,
		int itemNameLength,
		const Unicode::String & itemName,
		const std::string & locationName,
		const NetworkId & locationId
	);
	OnItemExpiredMessage(Archive::ReadIterator & source);
	~OnItemExpiredMessage();

	const NetworkId &          GetItemId()      const {return m_itemId.get();}
	const NetworkId &          GetOwnerId()     const {return m_ownerId.get();}
	int                        GetItemNameLength()   const {return m_itemNameLength.get();}
	const Unicode::String &    GetItemName()    const {return m_itemName.get();}
	const std::string &        GetLocationName()    const {return m_locationName.get();}
	const NetworkId &          GetLocationId()  const {return m_locationId.get();}
	int                        GetResponseId () const {return m_responseId.get();}
	int                        GetTrackId    () const {return m_trackId.get();}
	void                       SetTrackId    (int trackId) {m_trackId.set(trackId);}

protected:

private:
	Archive::AutoVariable<int>              m_responseId;
	Archive::AutoVariable<int>              m_trackId;
	Archive::AutoVariable<NetworkId>        m_ownerId;
	Archive::AutoVariable<NetworkId>        m_itemId;
	Archive::AutoVariable<int>              m_itemNameLength;
	Archive::AutoVariable<Unicode::String>  m_itemName;
	Archive::AutoVariable<std::string>      m_locationName;
	Archive::AutoVariable<NetworkId>        m_locationId;

};

#endif
