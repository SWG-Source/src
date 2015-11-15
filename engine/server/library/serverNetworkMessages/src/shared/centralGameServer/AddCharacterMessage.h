// ======================================================================
//
// AddCharacterMessage.h
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_AddCharacterMessage_H
#define INCLUDED_AddCharacterMessage_H

// ======================================================================

#include "sharedNetworkMessages/GameNetworkMessage.h"

// ======================================================================

/**
 * Add a character to an account.
 * Sent from:  Game Server
 * Sent to:  Database Server
 * Action:  Update the database to reflect that the specified character
 * belongs to the specified account.  After it is persisted, tell Central
 * that the character is ready.
 */
class AddCharacterMessage : public GameNetworkMessage
{
  public:
	AddCharacterMessage  (uint32 accountNumber, NetworkId objectId, uint32 process, const Unicode::String &name, bool special);
	AddCharacterMessage  (Archive::ReadIterator & source);
	~AddCharacterMessage ();

	uint32 getAccountNumber() const;
	NetworkId getObjectId() const;
	uint32 getProcess() const;
	const Unicode::String &getName() const;
	bool getSpecial() const;
	
  private:
	Archive::AutoVariable<uint32> m_accountNumber;
	Archive::AutoVariable<NetworkId> m_objectId;
	Archive::AutoVariable<uint32> m_process;
	Archive::AutoVariable<Unicode::String> m_name;
	Archive::AutoVariable<bool> m_special;
	
	AddCharacterMessage();
	AddCharacterMessage(const AddCharacterMessage&);
	AddCharacterMessage& operator= (const AddCharacterMessage&);
};

// ----------------------------------------------------------------------

inline uint32 AddCharacterMessage::getAccountNumber() const
{
	return m_accountNumber.get();
}

// ----------------------------------------------------------------------

inline NetworkId AddCharacterMessage::getObjectId() const
{
	return m_objectId.get();
}

// ----------------------------------------------------------------------

inline uint32 AddCharacterMessage::getProcess() const
{
	return m_process.get();
}

// ----------------------------------------------------------------------

inline const Unicode::String &AddCharacterMessage::getName() const
{
	return m_name.get();
}

// ----------------------------------------------------------------------

inline bool AddCharacterMessage::getSpecial() const
{
	return m_special.get();
}

// ======================================================================

#endif
