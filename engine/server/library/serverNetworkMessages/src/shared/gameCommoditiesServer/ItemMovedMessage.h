#ifndef ItemMovedMessage_H
#define ItemMovedMessage_H

#include "sharedNetworkMessages/GameNetworkMessage.h"
#include "sharedFoundation/NetworkId.h"

class ItemMovedMessage : public GameNetworkMessage
{

public:
	explicit ItemMovedMessage(
		const NetworkId &    playerId,
		const int            locationNameLength,
		const std::string &  locationName
	);

	ItemMovedMessage(Archive::ReadIterator & source);
	~ItemMovedMessage();

	const NetworkId &      GetPlayerId()           const {return m_playerId.get();}
	const int              GetLocationNameLength() const {return m_locationNameLength.get();}
	const std::string &    GetLocationName()       const {return m_locationName.get();}
		
protected:

private:
	Archive::AutoVariable<NetworkId>    m_playerId;
	Archive::AutoVariable<int>          m_locationNameLength;
	Archive::AutoVariable<std::string>  m_locationName;
};

#endif
