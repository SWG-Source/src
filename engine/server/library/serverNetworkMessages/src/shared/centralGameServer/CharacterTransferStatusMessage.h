//========================================================================
//
// CharacterTransferStatusMessage.h - tells Centralserver a new object is being created.
//
// copyright 2001 Sony Online Entertainment
//
//========================================================================

#ifndef	_INCLUDED_CharacterTransferStatusMessage_H
#define	_INCLUDED_CharacterTransferStatusMessage_H

//-----------------------------------------------------------------------

#include <string>
#include "sharedNetworkMessages/GameNetworkMessage.h"
#include "sharedMath/Vector.h"
#include "sharedFoundation/NetworkId.h"

//-----------------------------------------------------------------------

class CharacterTransferStatusMessage : public GameNetworkMessage
{
public:
	CharacterTransferStatusMessage  (unsigned int gameServerId, const NetworkId & toCharacterId, const std::string & statusMessage);
	CharacterTransferStatusMessage  (Archive::ReadIterator & source);
	~CharacterTransferStatusMessage ();

	unsigned int         getGameServerId   () const;
	const NetworkId &    getToCharacterId  () const;
	const std::string &  getStatusMessage  () const;

private:
	Archive::AutoVariable<unsigned int>  m_gameServerId;
	Archive::AutoVariable<NetworkId>     m_toCharacterId;
	Archive::AutoVariable<std::string>   m_statusMessage;
	
	CharacterTransferStatusMessage();
	CharacterTransferStatusMessage(const CharacterTransferStatusMessage&);
	CharacterTransferStatusMessage& operator= (const CharacterTransferStatusMessage&);
};

//-----------------------------------------------------------------------

inline unsigned int CharacterTransferStatusMessage::getGameServerId() const
{
	return m_gameServerId.get();
}

//-----------------------------------------------------------------------

inline const NetworkId & CharacterTransferStatusMessage::getToCharacterId() const
{
	return m_toCharacterId.get();
}

//-----------------------------------------------------------------------

inline const std::string & CharacterTransferStatusMessage::getStatusMessage() const
{
	return m_statusMessage.get();
}

//-----------------------------------------------------------------------

#endif	// _INCLUDED_CharacterTransferStatusMessage_H
