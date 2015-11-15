//========================================================================
//
// DownloadCharacterMessage.h - tells Centralserver a new object is being created.
//
// copyright 2001 Sony Online Entertainment
//
//========================================================================

#ifndef	_INCLUDED_DownloadCharacterMessage_H
#define	_INCLUDED_DownloadCharacterMessage_H

//-----------------------------------------------------------------------

#include <string>
#include "sharedNetworkMessages/GameNetworkMessage.h"
#include "sharedMath/Vector.h"
#include "sharedFoundation/NetworkId.h"

//-----------------------------------------------------------------------

class DownloadCharacterMessage : public GameNetworkMessage
{
public:
	DownloadCharacterMessage  (unsigned int stationId, unsigned int gameServerId, const NetworkId & toCharacterId, bool isAdmin);
	DownloadCharacterMessage  (Archive::ReadIterator & source);
	~DownloadCharacterMessage ();

	unsigned int       getStationId      () const;
	unsigned int       getGameServerId   () const;
	const NetworkId &  getToCharacterId  () const;
	bool               getIsAdmin        () const;

private:
	Archive::AutoVariable<unsigned int>  m_stationId;
	Archive::AutoVariable<unsigned int>  m_gameServerId;
	Archive::AutoVariable<NetworkId>     m_toCharacterId;
	Archive::AutoVariable<bool>          m_isAdmin;

	DownloadCharacterMessage();
	DownloadCharacterMessage(const DownloadCharacterMessage&);
	DownloadCharacterMessage& operator= (const DownloadCharacterMessage&);
};

//-----------------------------------------------------------------------

inline unsigned int DownloadCharacterMessage::getStationId() const
{
	return m_stationId.get();
}

//-----------------------------------------------------------------------

inline unsigned int DownloadCharacterMessage::getGameServerId() const
{
	return m_gameServerId.get();
}

//-----------------------------------------------------------------------

inline const NetworkId & DownloadCharacterMessage::getToCharacterId() const
{
	return m_toCharacterId.get();
}

//-----------------------------------------------------------------------

inline bool DownloadCharacterMessage::getIsAdmin() const
{
	return m_isAdmin.get();
}

//-----------------------------------------------------------------------

#endif	// _INCLUDED_DownloadCharacterMessage_H
