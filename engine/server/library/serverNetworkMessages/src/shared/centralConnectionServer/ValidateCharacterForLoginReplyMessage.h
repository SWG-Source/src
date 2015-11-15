// ======================================================================
//
// ValidateCharacterForLoginReplyMessage.h
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_ValidateCharacterForLoginReplyMessage_H
#define INCLUDED_ValidateCharacterForLoginReplyMessage_H

// ======================================================================

#include "Unicode.h"
#include "sharedFoundation/StationId.h"
#include "sharedMathArchive/VectorArchive.h"
#include "sharedNetworkMessages/GameNetworkMessage.h"
#include "unicodeArchive/UnicodeArchive.h"

// ======================================================================

/**
 * Sent from:  DatabaseProcess 
 * Sent to:  Central, which forwards to the appropriate ConnectionServer
 * Action:  Reply to ValidateCharacterForLoginMessage.  Tells the Connection
 *          Server whether the character is valid, and if so,
 *          where it is located in the world.
 */
class ValidateCharacterForLoginReplyMessage : public GameNetworkMessage
{
  public:
	ValidateCharacterForLoginReplyMessage(bool approved, StationId suid, const NetworkId &characterId, const NetworkId &containerId, const std::string &scene, const Vector &coordinates, const Unicode::String &characterName);
	ValidateCharacterForLoginReplyMessage(Archive::ReadIterator & source);

	bool                getApproved     () const;
	StationId           getSuid         () const;
	const NetworkId &   getCharacterId  () const;
	const NetworkId &   getContainerId  () const;
	const std::string & getScene        () const;
	const Vector &      getCoordinates  () const;
	const Unicode::String  &getCharacterName  () const;

  private:
	Archive::AutoVariable<bool> m_approved;
	Archive::AutoVariable<StationId> m_suid;
	Archive::AutoVariable<NetworkId> m_characterId;
	Archive::AutoVariable<NetworkId> m_containerId;
	Archive::AutoVariable<std::string> m_scene;
	Archive::AutoVariable<Vector> m_coordinates;
	Archive::AutoVariable<Unicode::String> m_characterName;
};

// ======================================================================

inline bool ValidateCharacterForLoginReplyMessage::getApproved() const
{
	return m_approved.get();
}

// ----------------------------------------------------------------------

inline StationId ValidateCharacterForLoginReplyMessage::getSuid() const
{
	return m_suid.get();
}

// ----------------------------------------------------------------------

inline const NetworkId & ValidateCharacterForLoginReplyMessage::getCharacterId() const
{
	return m_characterId.get();
}

// ----------------------------------------------------------------------

inline const NetworkId & ValidateCharacterForLoginReplyMessage::getContainerId() const
{
	return m_containerId.get();
}

// ----------------------------------------------------------------------

inline const std::string & ValidateCharacterForLoginReplyMessage::getScene() const
{
	return m_scene.get();
} 
// ----------------------------------------------------------------------

inline const Vector & ValidateCharacterForLoginReplyMessage::getCoordinates() const
{
	return m_coordinates.get();
}

// ----------------------------------------------------------------------

inline const Unicode::String  &ValidateCharacterForLoginReplyMessage::getCharacterName  () const
{
	return m_characterName.get();
}

// ======================================================================

#endif
 
