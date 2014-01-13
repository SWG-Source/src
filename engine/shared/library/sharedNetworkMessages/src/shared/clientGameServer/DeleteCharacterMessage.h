// ======================================================================
//
// DeleteCharacterMessage.h
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_DeleteCharacterMessage_H
#define INCLUDED_DeleteCharacterMessage_H

// ======================================================================

#include "sharedFoundation/NetworkIdArchive.h"
#include "sharedNetworkMessages/GameNetworkMessage.h"

//-----------------------------------------------------------------------

class DeleteCharacterMessage : public GameNetworkMessage
{
public:
	DeleteCharacterMessage(uint32 clusterId, const NetworkId &characterId);
	explicit DeleteCharacterMessage(Archive::ReadIterator & source);
	virtual ~DeleteCharacterMessage();

  public:
	uint32 getClusterId() const;
	const NetworkId &getCharacterId() const;
	
private:
	Archive::AutoVariable<uint32> m_clusterId;
	Archive::AutoVariable<NetworkId> m_characterId;

	DeleteCharacterMessage();
	DeleteCharacterMessage(const DeleteCharacterMessage&);
	DeleteCharacterMessage& operator= (const DeleteCharacterMessage&);
};

// ----------------------------------------------------------------------

inline uint32 DeleteCharacterMessage::getClusterId() const
{
	return m_clusterId.get();
}

// ----------------------------------------------------------------------

inline const NetworkId & DeleteCharacterMessage::getCharacterId() const
{
	return m_characterId.get();
}

// ======================================================================

#endif	// _INCLUDED_ClientCentralMessages_H
