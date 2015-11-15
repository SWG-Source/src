//========================================================================
//
// RenameCharacterMessage.h
//
// copyright 2001 Sony Online Entertainment
//
//========================================================================

#ifndef	_INCLUDED_RenameCharacterMessage_H
#define	_INCLUDED_RenameCharacterMessage_H

//-----------------------------------------------------------------------

#include "sharedNetworkMessages/GameNetworkMessage.h"
#include "unicodeArchive/UnicodeArchive.h"

//-----------------------------------------------------------------------

/**
 * Sent from:  Game Server
 * Sent to:    DBProcess
 * Action:     Rename the specified character-controlled object.  Name rules
 *             and uniqueness are not enforced.
 *
 * Sent from:  DBProcess
 * Send to:    LoginServer (via Central)
 * Action:     Update the character's name in the login database.
 */
class RenameCharacterMessage : public GameNetworkMessage
{
  public:
	RenameCharacterMessage  (const NetworkId &characterId, const Unicode::String &newName, const NetworkId &requestedBy);
	RenameCharacterMessage  (Archive::ReadIterator & source);
	~RenameCharacterMessage ();

  public:
	const NetworkId &       getCharacterId       () const;
	const Unicode::String & getNewName           () const;
	const NetworkId &       getRequestedBy       () const;
	
  private:
	Archive::AutoVariable<NetworkId>        m_characterId;
	Archive::AutoVariable<Unicode::String>  m_newName;
	Archive::AutoVariable<Unicode::String>  m_newNormalizedName;
	Archive::AutoVariable<NetworkId>        m_requestedBy;

  private:
	RenameCharacterMessage            (const RenameCharacterMessage&);
	RenameCharacterMessage& operator= (const RenameCharacterMessage&);
};

// ======================================================================

inline const NetworkId &RenameCharacterMessage::getCharacterId() const
{
	return m_characterId.get();
}

// ----------------------------------------------------------------------

inline const Unicode::String &RenameCharacterMessage::getNewName() const
{
	return m_newName.get();
}

// ----------------------------------------------------------------------

inline const NetworkId &RenameCharacterMessage::getRequestedBy() const
{
	return m_requestedBy.get();
}

// ======================================================================

class RenameCharacterMessageEx : public GameNetworkMessage
{
public:
	enum RenameCharacterMessageSource {RCMS_console_god_command, RCMS_cs_tool, RCMS_cts_rename, RCMS_player_request};

  public:
	RenameCharacterMessageEx  (RenameCharacterMessageSource renameCharacterMessageSource, uint32 stationId, const NetworkId &characterId, const Unicode::String &newName, const Unicode::String &oldName, const NetworkId &requestedBy);
	RenameCharacterMessageEx  (Archive::ReadIterator & source);
	~RenameCharacterMessageEx ();

  public:
	uint32                  getStationId         () const;
	const NetworkId &       getCharacterId       () const;
	const Unicode::String & getNewName           () const;
	const Unicode::String & getOldName           () const;
	const NetworkId &       getRequestedBy       () const;
	RenameCharacterMessageSource getRenameCharacterMessageSource () const;
	bool                    getLastNameChangeOnly() const;

  private:
	Archive::AutoVariable<uint32>           m_stationId;
	Archive::AutoVariable<NetworkId>        m_characterId;
	Archive::AutoVariable<Unicode::String>  m_newName;
	Archive::AutoVariable<Unicode::String>  m_oldName;
	Archive::AutoVariable<NetworkId>        m_requestedBy;
	Archive::AutoVariable<int8>             m_renameCharacterMessageSource;
	Archive::AutoVariable<bool>             m_lastNameChangeOnly;

  private:
	RenameCharacterMessageEx            (const RenameCharacterMessageEx&);
	RenameCharacterMessageEx& operator= (const RenameCharacterMessageEx&);
};

// ======================================================================

inline uint32 RenameCharacterMessageEx::getStationId() const
{
	return m_stationId.get();
}

// ----------------------------------------------------------------------

inline const NetworkId &RenameCharacterMessageEx::getCharacterId() const
{
	return m_characterId.get();
}

// ----------------------------------------------------------------------

inline const Unicode::String &RenameCharacterMessageEx::getNewName() const
{
	return m_newName.get();
}

// ----------------------------------------------------------------------

inline const Unicode::String &RenameCharacterMessageEx::getOldName() const
{
	return m_oldName.get();
}

// ----------------------------------------------------------------------

inline const NetworkId &RenameCharacterMessageEx::getRequestedBy() const
{
	return m_requestedBy.get();
}

// ----------------------------------------------------------------------

inline RenameCharacterMessageEx::RenameCharacterMessageSource RenameCharacterMessageEx::getRenameCharacterMessageSource () const
{
	return static_cast<RenameCharacterMessageSource>(m_renameCharacterMessageSource.get());
}

// ----------------------------------------------------------------------

inline bool RenameCharacterMessageEx::getLastNameChangeOnly() const
{
	return m_lastNameChangeOnly.get();
}

// ======================================================================

#endif
