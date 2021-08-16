// ======================================================================
//
// ClientPermissionsMessage.h
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_ClientPermissionsMessage_H
#define INCLUDED_ClientPermissionsMessage_H

// ======================================================================

#include "sharedFoundation/NetworkIdArchive.h"
#include "sharedNetworkMessages/GameNetworkMessage.h"

//-----------------------------------------------------------------------

/**
 * Sent from:  Connection Server
 * Sent to:    Client
 * Action:     Tells the client what permissions it has on the server its
 *             connected to.
 */
class ClientPermissionsMessage : public GameNetworkMessage
{
  public:
	ClientPermissionsMessage(bool canLogin, bool canCreateRegularCharacter, bool canCreateJediCharacter, bool canSkipTutorial, bool isAdmin);
	explicit ClientPermissionsMessage(Archive::ReadIterator & source);
	virtual ~ClientPermissionsMessage();

  public:
	bool getCanLogin() const;
	bool getCanCreateRegularCharacter() const;
	bool getCanCreateJediCharacter() const;
	bool getCanSkipTutorial() const;
	bool getIsAdmin() const;
	
  private:
	Archive::AutoVariable<bool> m_canLogin;
	Archive::AutoVariable<bool> m_canCreateRegularCharacter;
	Archive::AutoVariable<bool> m_canCreateJediCharacter;
	Archive::AutoVariable<bool> m_canSkipTutorial;
	Archive::AutoVariable<bool> m_isAdmin;

	ClientPermissionsMessage();
	ClientPermissionsMessage(const ClientPermissionsMessage&);
	ClientPermissionsMessage& operator= (const ClientPermissionsMessage&);
};

// ----------------------------------------------------------------------

inline bool ClientPermissionsMessage::getCanLogin() const
{
	return m_canLogin.get();
}

// ----------------------------------------------------------------------

inline bool ClientPermissionsMessage::getCanCreateRegularCharacter() const
{
	return m_canCreateRegularCharacter.get();
}

// ----------------------------------------------------------------------

inline bool ClientPermissionsMessage::getCanCreateJediCharacter() const
{
	return m_canCreateJediCharacter.get();
}

// ----------------------------------------------------------------------

inline bool ClientPermissionsMessage::getCanSkipTutorial() const
{
	return m_canSkipTutorial.get();
}

// ----------------------------------------------------------------------

inline bool ClientPermissionsMessage::getIsAdmin() const
{
    return m_isAdmin.get();
}

// ======================================================================

#endif	// _INCLUDED_ClientCentralMessages_H
