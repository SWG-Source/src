// SceneTransferMessages.cpp
// copyright 2001 Sony Online Entertainment
//

#include "serverNetworkMessages/FirstServerNetworkMessages.h"

#include "serverNetworkMessages/SceneTransferMessages.h"

//-----------------------------------------------------------------------

RequestSceneTransfer::RequestSceneTransfer(const NetworkId &oid, const std::string & sceneName, uint32 sourceGameServer, const Vector &position_p, const Vector &position_w, const NetworkId &containerId, const char * scriptCallback) :
	GameNetworkMessage("RequestSceneTransfer"),
	m_oid(oid),
	m_sceneName(sceneName),
	m_sourceGameServer(sourceGameServer),
	m_position_p(position_p),
	m_position_w(position_w),
	m_containerId(containerId),
	m_containerName(),
	m_scriptCallback()
{
	if (scriptCallback != nullptr)
		m_scriptCallback.set(scriptCallback);

	addVariable(m_oid);
	addVariable(m_sceneName);
	addVariable(m_sourceGameServer);
	addVariable(m_position_p);
	addVariable(m_position_w);
	addVariable(m_containerId);
	addVariable(m_containerName);
	addVariable(m_scriptCallback);
}

//-----------------------------------------------------------------------

RequestSceneTransfer::RequestSceneTransfer(const NetworkId &oid, const std::string & sceneName, uint32 sourceGameServer, const Vector &position_p, const Vector &position_w, const NetworkId &buildingId, const std::string &containerName, const char * scriptCallback) :
	GameNetworkMessage("RequestSceneTransfer"),
	m_oid(oid),
	m_sceneName(sceneName),
	m_sourceGameServer(sourceGameServer),
	m_position_p(position_p),
	m_position_w(position_w),
	m_containerId(buildingId),
	m_containerName(containerName),
	m_scriptCallback()
{
	if (scriptCallback != nullptr)
		m_scriptCallback.set(scriptCallback);

	addVariable(m_oid);
	addVariable(m_sceneName);
	addVariable(m_sourceGameServer);
	addVariable(m_position_p);
	addVariable(m_position_w);
	addVariable(m_containerId);
	addVariable(m_containerName);
	addVariable(m_scriptCallback);
}

//-----------------------------------------------------------------------

RequestSceneTransfer::RequestSceneTransfer(Archive::ReadIterator & source) :
	GameNetworkMessage("RequestSceneTransfer"),
	m_oid(NetworkId::cms_invalid),
	m_sceneName(),
	m_sourceGameServer(),
	m_position_p(),
	m_position_w(),
	m_containerId(),
	m_containerName(),
	m_scriptCallback()
{
	addVariable(m_oid);
	addVariable(m_sceneName);
	addVariable(m_sourceGameServer);
	addVariable(m_position_p);
	addVariable(m_position_w);
	addVariable(m_containerId);
	addVariable(m_containerName);
	addVariable(m_scriptCallback);
	unpack(source);
}

//-----------------------------------------------------------------------

RequestSceneTransfer::~RequestSceneTransfer()
{
}

// ======================================================================

SceneTransferMessage::SceneTransferMessage(const RequestSceneTransfer &source, uint32 destinationGameServer) :
	GameNetworkMessage("SceneTransferMessage"),
	m_oid(source.getNetworkId()),
	m_sceneName(source.getSceneName()),
	m_sourceGameServer(source.getSourceGameServer()),
	m_position_p(source.getPosition_p()),
	m_position_w(source.getPosition_w()),
	m_containerId(source.getContainerId()),
	m_containerName(source.getContainerName()),
	m_scriptCallback(source.getScriptCallback()),
	m_destinationGameServer(destinationGameServer)
{
	addVariable(m_oid);
	addVariable(m_sceneName);
	addVariable(m_sourceGameServer);
	addVariable(m_position_p);
	addVariable(m_position_w);
	addVariable(m_containerId);
	addVariable(m_containerName);
	addVariable(m_scriptCallback);
	addVariable(m_destinationGameServer);
}

//-----------------------------------------------------------------------

SceneTransferMessage::SceneTransferMessage(Archive::ReadIterator & source) :
	GameNetworkMessage("SceneTransferMessage"),
	m_oid(NetworkId::cms_invalid),
	m_sceneName(),
	m_sourceGameServer(),
	m_position_p(),
	m_position_w(),
	m_containerId(),
	m_containerName(),
	m_scriptCallback(),
	m_destinationGameServer()
{
	addVariable(m_oid);
	addVariable(m_sceneName);
	addVariable(m_sourceGameServer);
	addVariable(m_position_p);
	addVariable(m_position_w);
	addVariable(m_containerId);
	addVariable(m_containerName);
	addVariable(m_scriptCallback);
	addVariable(m_destinationGameServer);

	unpack(source);
}

//-----------------------------------------------------------------------

SceneTransferMessage::~SceneTransferMessage()
{
}

// ======================================================================

