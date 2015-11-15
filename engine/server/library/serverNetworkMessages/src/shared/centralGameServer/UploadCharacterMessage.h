//========================================================================
//
// UploadCharacterMessage.h - tells Centralserver a new object is being created.
//
// copyright 2001 Sony Online Entertainment
//
//========================================================================

#ifndef	_INCLUDED_UploadCharacterMessage_H
#define	_INCLUDED_UploadCharacterMessage_H

//-----------------------------------------------------------------------

#include <string>
#include "sharedNetworkMessages/GameNetworkMessage.h"
#include "sharedMath/Vector.h"

//-----------------------------------------------------------------------

class UploadCharacterMessage : public GameNetworkMessage
{
public:
	UploadCharacterMessage  (unsigned int stationId, const std::string & packedCharacterData, const unsigned int fromGameServerId, const NetworkId & fromCharacterId, bool isAdmin);
	UploadCharacterMessage  (Archive::ReadIterator & source);
	~UploadCharacterMessage ();

	unsigned int         getStationId            () const;
	const std::string &  getPackedCharacterData  () const;
	unsigned int         getFromGameServerId     () const;
	const NetworkId &    getFromCharacterId      () const;
	bool                 getIsAdmin              () const;

private:
	Archive::AutoVariable<unsigned int>  m_stationId;
	Archive::AutoVariable<std::string>   m_packedCharacterData;
	Archive::AutoVariable<unsigned int>  m_fromGameServerId;
	Archive::AutoVariable<NetworkId>     m_fromCharacterId;
	Archive::AutoVariable<bool>          m_isAdmin;

	UploadCharacterMessage();
	UploadCharacterMessage(const UploadCharacterMessage&);
	UploadCharacterMessage& operator= (const UploadCharacterMessage&);
};

//-----------------------------------------------------------------------

inline unsigned int UploadCharacterMessage::getStationId() const
{
	return m_stationId.get();
}

//-----------------------------------------------------------------------

inline unsigned int UploadCharacterMessage::getFromGameServerId() const
{
	return m_fromGameServerId.get();
}

//-----------------------------------------------------------------------

inline const NetworkId & UploadCharacterMessage::getFromCharacterId() const
{
	return m_fromCharacterId.get();
}

//-----------------------------------------------------------------------

inline const std::string & UploadCharacterMessage::getPackedCharacterData() const
{
	return m_packedCharacterData.get();
}

//-----------------------------------------------------------------------

inline bool UploadCharacterMessage::getIsAdmin() const
{
	return m_isAdmin.get();
}

//-----------------------------------------------------------------------

#endif	// _INCLUDED_UploadCharacterMessage_H
