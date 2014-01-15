// ======================================================================
//
// RequestGameServerForLoginMessage.h
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_RequestGameServerForLoginMessage_H
#define INCLUDED_RequestGameServerForLoginMessage_H

// ======================================================================

#include "sharedNetworkMessages/GameNetworkMessage.h"

// ======================================================================

/** 
 * Pick a game server for login
 *
 * Sent from:  ConnectionServer 
 * Sent to:  PlanetServer (via Central)
 * Action:  Pick a good game server to use based on the coordinates
 * specified.  Reply with GameServerForLoginMessage.
 */
class RequestGameServerForLoginMessage : public GameNetworkMessage
{
  public:
	RequestGameServerForLoginMessage  (uint32 stationId, const NetworkId &characterId, const NetworkId &containerId, const std::string &scene, const Vector &coordinates, bool forCtsSourceCharacter);
	RequestGameServerForLoginMessage  (Archive::ReadIterator & source);
	~RequestGameServerForLoginMessage ();

  public:
	const std::string &  getScene() const;
	const Vector &       getCoordinates() const;
	uint32               getStationId() const;
	const NetworkId &    getContainerId() const;
	const NetworkId &    getCharacterId() const;
	bool                 getForCtsSourceCharacter() const;
	
  private:
	Archive::AutoVariable<uint32> m_stationId;
	Archive::AutoVariable<NetworkId> m_characterId;
	Archive::AutoVariable<NetworkId> m_containerId;
	Archive::AutoVariable<std::string> m_scene;
	Archive::AutoVariable<Vector> m_coordinates;
	Archive::AutoVariable<bool> m_forCtsSourceCharacter;

  private:
	RequestGameServerForLoginMessage();
	RequestGameServerForLoginMessage(const RequestGameServerForLoginMessage&);
	RequestGameServerForLoginMessage& operator= (const RequestGameServerForLoginMessage&);
};

// ======================================================================

inline const std::string &RequestGameServerForLoginMessage::getScene() const
{
	return m_scene.get();
}

// ----------------------------------------------------------------------

inline const Vector &RequestGameServerForLoginMessage::getCoordinates() const
{
	return m_coordinates.get();
}

// ----------------------------------------------------------------------

inline uint32 RequestGameServerForLoginMessage::getStationId() const
{
	return m_stationId.get();
}

// ----------------------------------------------------------------------

inline const NetworkId &RequestGameServerForLoginMessage::getContainerId() const
{
	return m_containerId.get();
}

// ----------------------------------------------------------------------

inline const NetworkId &RequestGameServerForLoginMessage::getCharacterId() const
{
	return m_characterId.get();
}

// ----------------------------------------------------------------------

inline bool RequestGameServerForLoginMessage::getForCtsSourceCharacter() const
{
	return m_forCtsSourceCharacter.get();
}

// ======================================================================

#endif
