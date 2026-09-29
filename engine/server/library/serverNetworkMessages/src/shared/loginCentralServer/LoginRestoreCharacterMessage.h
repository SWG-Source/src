// ======================================================================
//
// LoginRestoreCharacterMessage.h
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_LoginRestoreCharacterMessage_H
#define INCLUDED_LoginRestoreCharacterMessage_H

// ======================================================================

#include "sharedFoundation/StationId.h"
#include "sharedNetworkMessages/GameNetworkMessage.h"

// ======================================================================

/** 
 * Sent From:  Central
 * Sent To:    LoginServer
 * Action:     Restore the specified character to the login database
 */

class LoginRestoreCharacterMessage : public GameNetworkMessage
{
  public:
    LoginRestoreCharacterMessage (const std::string &whoRequested, const NetworkId &characterId, StationId account, const Unicode::String &characterName, uint32 templateId, bool jedi);
    LoginRestoreCharacterMessage (Archive::ReadIterator & source);
    virtual ~LoginRestoreCharacterMessage ();

  public:
	const std::string &   getWhoRequested() const;
	const NetworkId &     getCharacterId() const;
	StationId             getAccount() const;
	const Unicode::String getCharacterName() const;
	uint32                   getTemplateId() const;
	bool                  getJedi() const;
  	
  private:
	Archive::AutoVariable<std::string>     m_whoRequested;
	Archive::AutoVariable<NetworkId>       m_characterId;
    Archive::AutoVariable<StationId>       m_account;
	Archive::AutoVariable<Unicode::String> m_characterName;
	Archive::AutoVariable<uint32>             m_templateId;
	Archive::AutoVariable<bool>            m_jedi;
  
  private: // disable:
    LoginRestoreCharacterMessage();
    LoginRestoreCharacterMessage(const LoginRestoreCharacterMessage&);
    LoginRestoreCharacterMessage& operator= (const LoginRestoreCharacterMessage&);

};

// ======================================================================

inline const std::string & LoginRestoreCharacterMessage::getWhoRequested() const
{
	return m_whoRequested.get();
}

// ----------------------------------------------------------------------

inline const NetworkId & LoginRestoreCharacterMessage::getCharacterId() const
{
	return m_characterId.get();
}

// ----------------------------------------------------------------------

inline StationId LoginRestoreCharacterMessage::getAccount() const
{
	return m_account.get();
}

// ----------------------------------------------------------------------

inline const Unicode::String LoginRestoreCharacterMessage::getCharacterName() const
{
	return m_characterName.get();
}

// ----------------------------------------------------------------------

inline uint32 LoginRestoreCharacterMessage::getTemplateId() const
{
	return m_templateId.get();
}

// ----------------------------------------------------------------------

inline bool LoginRestoreCharacterMessage::getJedi() const
{
	return m_jedi.get();
}
 
// ======================================================================

#endif
