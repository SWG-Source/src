// SceneTransferMessages.h
// copyright 2001 Sony Online Entertainment
//

#ifndef _INCLUDED_SceneTransferMessages_H
#define _INCLUDED_SceneTransferMessages_H

#include "sharedNetworkMessages/GameNetworkMessage.h"

//-----------------------------------------------------------------------

/** 
 * Initiate the process of transferring an object to a different scene.
 *
 * Sent from:  Gameserver 
 * Sent to:  PlanetServer for new scene (via Central)
 * Action:  Select a game server to receive the object.  Reply with
 * SceneTransferMessage.
 */
class RequestSceneTransfer : public GameNetworkMessage
{
public:
    
	RequestSceneTransfer(const NetworkId &oid, const std::string & sceneName, uint32 sourceGameServer, const Vector &position_p, const Vector &position_w, const NetworkId &containerId, const char * scriptCallback = nullptr);
	RequestSceneTransfer(const NetworkId &oid, const std::string & sceneName, uint32 sourceGameServer, const Vector &position_p, const Vector &position_w, const NetworkId &buildingId, const std::string &containerName, const char * scriptCallback = nullptr);
	RequestSceneTransfer(Archive::ReadIterator & source);
	~RequestSceneTransfer();

	const NetworkId &         getNetworkId() const;
	const std::string &       getSceneName() const;
	uint32                    getSourceGameServer() const;
	const Vector &            getPosition_p() const;
	const Vector &            getPosition_w() const;
	const NetworkId &         getContainerId() const;
	const std::string &       getContainerName() const;
	const std::string &       getScriptCallback() const;
        
private:

	Archive::AutoVariable<NetworkId>    m_oid;
	Archive::AutoVariable<std::string>  m_sceneName;
	Archive::AutoVariable<uint32>       m_sourceGameServer;
	Archive::AutoVariable<Vector>       m_position_p;
	Archive::AutoVariable<Vector>       m_position_w;
	Archive::AutoVariable<NetworkId>    m_containerId;
	Archive::AutoVariable<std::string>  m_containerName;
	Archive::AutoVariable<std::string>  m_scriptCallback;
    
private:
	RequestSceneTransfer();
	RequestSceneTransfer(const RequestSceneTransfer&);
	RequestSceneTransfer & operator= (const RequestSceneTransfer&);
};

//-----------------------------------------------------------------------

inline const NetworkId & RequestSceneTransfer::getNetworkId() const
{
	return m_oid.get();
}

//-----------------------------------------------------------------------

inline const std::string & RequestSceneTransfer::getSceneName() const
{
	return m_sceneName.get();
}

// ----------------------------------------------------------------------

inline uint32 RequestSceneTransfer::getSourceGameServer() const
{
	return m_sourceGameServer.get();
}

//-----------------------------------------------------------------------

inline const Vector & RequestSceneTransfer::getPosition_p() const
{
	return m_position_p.get();
}

//-----------------------------------------------------------------------

inline const Vector & RequestSceneTransfer::getPosition_w() const
{
	return m_position_w.get();
}

// ----------------------------------------------------------------------

inline const NetworkId & RequestSceneTransfer::getContainerId() const
{
	return m_containerId.get();
}

// ----------------------------------------------------------------------

inline const std::string & RequestSceneTransfer::getContainerName() const
{
	return m_containerName.get();
}

// ----------------------------------------------------------------------

inline const std::string & RequestSceneTransfer::getScriptCallback() const
{
	return m_scriptCallback.get();
}

// ======================================================================

/** 
 * Tell the Gameserver to transfer an object to a scene.  Usually (always?)
 * sent in reply to RequestSceneTransfer
 *
 * Sent from:  PlanetServer
 * Sent to:  Gameserver (via Central)
 * Action:  Create a proxy on the specified game server, then make the
 * proxy authoritative.  Remove all other proxies
 */
class SceneTransferMessage : public GameNetworkMessage
{
public:
	SceneTransferMessage(const RequestSceneTransfer &source, uint32 destinationGameServer);
	SceneTransferMessage(Archive::ReadIterator & source);
	~SceneTransferMessage();

public:
	const NetworkId &   getNetworkId() const;
	const std::string & getSceneName() const;
	uint32              getSourceGameServer() const;
	const Vector &      getPosition_p() const;
	const Vector &      getPosition_w() const;
	const NetworkId &   getContainerId() const;
	const std::string & getContainerName() const;
	const std::string & getScriptCallback() const;
	uint32              getDestinationGameServer() const;
	
private:
	Archive::AutoVariable<NetworkId>    m_oid;
	Archive::AutoVariable<std::string>  m_sceneName;
	Archive::AutoVariable<uint32>       m_sourceGameServer;
	Archive::AutoVariable<Vector>       m_position_p;
	Archive::AutoVariable<Vector>       m_position_w;
	Archive::AutoVariable<NetworkId>    m_containerId;
	Archive::AutoVariable<std::string>  m_containerName;
	Archive::AutoVariable<std::string>  m_scriptCallback;
	Archive::AutoVariable<uint32>       m_destinationGameServer;
    
private:
	SceneTransferMessage();
	SceneTransferMessage(const SceneTransferMessage&);
	SceneTransferMessage & operator= (const SceneTransferMessage&);
};

// ----------------------------------------------------------------------

inline const NetworkId & SceneTransferMessage::getNetworkId() const
{
	return m_oid.get();
}

// ----------------------------------------------------------------------

inline const std::string &SceneTransferMessage::getSceneName() const
{
	return m_sceneName.get();
}

// ----------------------------------------------------------------------

inline uint32 SceneTransferMessage::getSourceGameServer() const
{
	return m_sourceGameServer.get();
}

// ----------------------------------------------------------------------

inline const Vector &SceneTransferMessage::getPosition_p() const
{
	return m_position_p.get();
}

// ----------------------------------------------------------------------

inline const Vector &SceneTransferMessage::getPosition_w() const
{
	return m_position_w.get();
}

// ----------------------------------------------------------------------

inline const NetworkId &SceneTransferMessage::getContainerId() const
{
	return m_containerId.get();
}

// ----------------------------------------------------------------------

inline const std::string &SceneTransferMessage::getContainerName() const
{
	return m_containerName.get();
}

// ----------------------------------------------------------------------

inline const std::string & SceneTransferMessage::getScriptCallback() const
{
	return m_scriptCallback.get();
}

// ----------------------------------------------------------------------

inline uint32 SceneTransferMessage::getDestinationGameServer() const
{
	return m_destinationGameServer.get();
}

// ======================================================================

#endif //_INCLUDED_SceneTransferMessages_H

