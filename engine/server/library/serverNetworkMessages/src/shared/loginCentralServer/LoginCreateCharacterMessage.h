// ======================================================================
//
// LoginCreateCharacterMessage.h
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_LoginCreateCharacterMessage_H
#define INCLUDED_LoginCreateCharacterMessage_H

// ======================================================================

#include "sharedFoundation/StationId.h"
#include "sharedNetworkMessages/GameNetworkMessage.h"

// ======================================================================

/** 
 * Sent From:  Central
 * Sent To:    LoginServer
 * Action:     Add the specified character to the login database and
 *             reply with LoginCreateCharacterAckMessage
 */

class LoginCreateCharacterMessage : public GameNetworkMessage
{
  public:
    LoginCreateCharacterMessage (const StationId &stationId, const Unicode::String &characterName, const NetworkId &characterObjectId, int templateId, bool jedi);
    LoginCreateCharacterMessage (Archive::ReadIterator & source);
    virtual ~LoginCreateCharacterMessage ();

  public:
	StationId           getStationId() const;
	const Unicode::String & getCharacterName() const;
	const NetworkId &   getCharacterObjectId() const;
	int                 getTemplateId() const;
	bool                getJedi() const;
	
  private:
    Archive::AutoVariable<StationId> m_stationId;
	Archive::AutoVariable<Unicode::String> m_characterName;
	Archive::AutoVariable<NetworkId> m_characterObjectId;
	Archive::AutoVariable<int> m_templateId;
	Archive::AutoVariable<bool> m_jedi;
  
  private: // disable:
    LoginCreateCharacterMessage();
    LoginCreateCharacterMessage(const LoginCreateCharacterMessage&);
    LoginCreateCharacterMessage& operator= (const LoginCreateCharacterMessage&);

};

// ======================================================================

inline StationId LoginCreateCharacterMessage::getStationId() const
{
	return m_stationId.get();
}

// ----------------------------------------------------------------------

inline const Unicode::String & LoginCreateCharacterMessage::getCharacterName() const
{
	return m_characterName.get();
}

// ----------------------------------------------------------------------

inline const NetworkId & LoginCreateCharacterMessage::getCharacterObjectId() const
{
	return m_characterObjectId.get();
}

// ----------------------------------------------------------------------

inline int LoginCreateCharacterMessage::getTemplateId() const
{
	return m_templateId.get();
}

// ----------------------------------------------------------------------

inline bool LoginCreateCharacterMessage::getJedi() const
{
	return m_jedi.get();
}

// ======================================================================

#endif
