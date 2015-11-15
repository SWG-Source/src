//========================================================================
//
// AddCharacterMessage.cpp - tells Centralserver we want all the objects in a chunk.
//
// copyright 2001 Sony Online Entertainment
//
//========================================================================

#include "serverNetworkMessages/FirstServerNetworkMessages.h"
#include "serverNetworkMessages/AddCharacterMessage.h"

#include "serverNetworkMessages/CentralGameServerMessages.h"
#include "unicodeArchive/UnicodeArchive.h"

// ======================================================================

AddCharacterMessage::AddCharacterMessage  (uint32 accountNumber, NetworkId objectId, uint32 process, const Unicode::String &name, bool special) :
		GameNetworkMessage("AddCharacterMessage"),
		m_accountNumber(accountNumber),
		m_objectId(objectId),
		m_process(process),
		m_name(name),
		m_special(special)
{
	addVariable(m_accountNumber);
	addVariable(m_objectId);
	addVariable(m_process);
	addVariable(m_name);
	addVariable(m_special);
}

//-----------------------------------------------------------------------

AddCharacterMessage::AddCharacterMessage(Archive::ReadIterator & source) :
		GameNetworkMessage("AddCharacterMessage"),
		m_accountNumber(),
		m_objectId(),
		m_process(),
		m_name(),
		m_special()
{
	addVariable(m_accountNumber);
	addVariable(m_objectId);
	addVariable(m_process);
	addVariable(m_name);
	addVariable(m_special);
	unpack(source);
}

//-----------------------------------------------------------------------

AddCharacterMessage::~AddCharacterMessage()
{
}

// ======================================================================
