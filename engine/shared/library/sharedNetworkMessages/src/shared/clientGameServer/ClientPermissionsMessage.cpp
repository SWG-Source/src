// ======================================================================
//
// ClientPermissionsMessage.cpp
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#include "sharedNetworkMessages/FirstSharedNetworkMessages.h"
#include "sharedNetworkMessages/ClientPermissionsMessage.h"

// ======================================================================

ClientPermissionsMessage::ClientPermissionsMessage(bool canLogin, bool canCreateRegularCharacter, bool canCreateJediCharacter, bool canSkipTutorial) :
		GameNetworkMessage("ClientPermissionsMessage"),
		m_canLogin(canLogin),
		m_canCreateRegularCharacter(canCreateRegularCharacter),
		m_canCreateJediCharacter(canCreateJediCharacter),
		m_canSkipTutorial(canSkipTutorial)
{
	addVariable(m_canLogin);
	addVariable(m_canCreateRegularCharacter);
	addVariable(m_canCreateJediCharacter);
	addVariable(m_canSkipTutorial);
}

//-----------------------------------------------------------------------

ClientPermissionsMessage::ClientPermissionsMessage(Archive::ReadIterator & source) :
		GameNetworkMessage("ClientPermissionsMessage"),
		m_canLogin(),
		m_canCreateRegularCharacter(),
		m_canCreateJediCharacter(),
		m_canSkipTutorial()
{
	addVariable(m_canLogin);
	addVariable(m_canCreateRegularCharacter);
	addVariable(m_canCreateJediCharacter);
	addVariable(m_canSkipTutorial);

	unpack(source);
}

// ----------------------------------------------------------------------

ClientPermissionsMessage::~ClientPermissionsMessage()
{
}

// ======================================================================
