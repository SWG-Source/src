// GameConnectionServerMessages.cpp
// copyright 2001 Verant Interactive



//-----------------------------------------------------------------------

#include "serverNetworkMessages/FirstServerNetworkMessages.h"
#include "serverNetworkMessages/GameConnectionServerMessages.h"

#include "sharedFoundation/ConstCharCrcString.h"
#include "sharedNetwork/ConfigSharedNetwork.h"
#include "sharedNetwork/NetworkHandler.h"
#include "sharedMathArchive/VectorArchive.h"

//-----------------------------------------------------------------------

static ConstCharCrcString s_gameClientMessageName("GameClientMessage");

//-----------------------------------------------------------------------

NewGameServer::NewGameServer(const std::string & sceneName, uint32 serverId) : 
		GameNetworkMessage("NewGameServer"),
		m_sceneName(sceneName),
		m_serverId(serverId)
{
	addVariable(m_sceneName);
	addVariable(m_serverId);
}
//-----------------------------------------------------------------------

NewGameServer::NewGameServer (Archive::ReadIterator & source) : 
		GameNetworkMessage("NewGameServer"),
		m_sceneName(),
		m_serverId(0)
{
	addVariable(m_sceneName);
	addVariable(m_serverId);
	unpack(source);
}
//-----------------------------------------------------------------------

NewGameServer::~NewGameServer()
{
}

// ======================================================================

TransferControlMessage::TransferControlMessage(NetworkId oid, uint32 newGameServerId, bool skipLoadScreen, const std::vector<NetworkId> &observedObjects) :
		GameNetworkMessage("TransferControlMessage"),
		m_oid(oid),
		m_gameServerId(newGameServerId),
		m_skipLoadScreen(skipLoadScreen),
		m_observedObjects()
{
	m_observedObjects.set(observedObjects);
	
	addVariable(m_oid);
	addVariable(m_gameServerId);
	addVariable(m_skipLoadScreen);
	addVariable(m_observedObjects);
}

//-----------------------------------------------------------------------

TransferControlMessage::TransferControlMessage(Archive::ReadIterator & source) :
GameNetworkMessage("TransferControlMessage"),
m_oid(NetworkId::cms_invalid),
m_gameServerId(0),
m_skipLoadScreen(true)
{
	addVariable(m_oid);
	addVariable(m_gameServerId);
	addVariable(m_skipLoadScreen);
	addVariable(m_observedObjects);

	unpack(source);
}

//-----------------------------------------------------------------------

TransferControlMessage::~TransferControlMessage()
{
}

//-----------------------------------------------------------------------
ControlAssumed::ControlAssumed(NetworkId oid, const std::string & newSceneName, const bool skipLoadScreen,
							   const Vector &startPos, const float startYaw, const std::string & templateName,
							   const int64 t) : 
		GameNetworkMessage("ControlAssumed"),
		m_oid(oid),
		m_sceneName(newSceneName),
		m_skipLoadScreen(skipLoadScreen),
		m_startPosition(startPos),
		m_startYaw(startYaw),
		m_templateName(templateName),
		m_timeSeconds(t)
{
	addVariable(m_oid);
	addVariable(m_sceneName);
	addVariable(m_skipLoadScreen);
	addVariable(m_startPosition);
	addVariable(m_startYaw);
	addVariable(m_templateName);
	addVariable(m_timeSeconds);
}
//-----------------------------------------------------------------------

ControlAssumed::ControlAssumed (Archive::ReadIterator & source) : 
		GameNetworkMessage("ControlAssumed"),
		m_oid(NetworkId::cms_invalid),
		m_sceneName(),
		m_skipLoadScreen(false),
		m_startPosition(),
		m_startYaw(0.0f),
		m_templateName(),
		m_timeSeconds(0)
{
	addVariable(m_oid);
	addVariable(m_sceneName);
	addVariable(m_skipLoadScreen);
	addVariable(m_startPosition);
	addVariable(m_startYaw);
	addVariable(m_templateName);
	addVariable(m_timeSeconds);
	unpack(source);
}
//-----------------------------------------------------------------------

ControlAssumed::~ControlAssumed()
{
}
//-----------------------------------------------------------------------

DropClient::DropClient(NetworkId oid) : 
		GameNetworkMessage("DropClient"),
		m_oid(oid),
		m_immediate(false)
{
	addVariable(m_oid);
	addVariable(m_immediate);
}
//-----------------------------------------------------------------------

DropClient::DropClient (Archive::ReadIterator & source) : 
		GameNetworkMessage("DropClient"),
		m_oid(NetworkId::cms_invalid),
		m_immediate(false)
{
	addVariable(m_oid);
	addVariable(m_immediate);
	unpack(source);
}
//-----------------------------------------------------------------------

DropClient::~DropClient()
{
}

//-----------------------------------------------------------------------

KickPlayer::KickPlayer(const NetworkId& oid, const std::string& reason) : 
	GameNetworkMessage("KickPlayer"),
	m_oid(oid),
	m_reason(reason)
{
	addVariable(m_oid);
	addVariable(m_reason);
}

//-----------------------------------------------------------------------

KickPlayer::KickPlayer(Archive::ReadIterator &source) : 
	GameNetworkMessage("KickPlayer"),
	m_oid(NetworkId::cms_invalid),
	m_reason("")
{
	addVariable(m_oid);
	addVariable(m_reason);
	unpack(source);
}

//-----------------------------------------------------------------------

KickPlayer::~KickPlayer()
{
}

//-----------------------------------------------------------------------

GameClientMessage::GameClientMessage (const std::vector<NetworkId> & d, bool newReliable, const GameNetworkMessage& msg) :
	GameNetworkMessage(s_gameClientMessageName),
	distributionList(),
	reliable(newReliable),
	byteStream()
{
	distributionList.set(d);
	addVariable(distributionList);
	addVariable(reliable);
	addVariable(byteStream);
	Archive::ByteStream bs;
	msg.pack(bs);
	byteStream.set(bs);

	if (ConfigSharedNetwork::getReportMessages())
		NetworkHandler::reportMessage("send.GameClientMessage." + msg.getCmdName(), bs.getSize());
}

//-----------------------------------------------------------------------

GameClientMessage::GameClientMessage(const std::vector<NetworkId> & d, bool newReliable, Archive::ReadIterator & msg) :
	GameNetworkMessage(s_gameClientMessageName),
	distributionList(),
	reliable(newReliable),
	byteStream(Archive::ByteStream(msg.getBuffer(), msg.getSize()))
{
	distributionList.set(d);
	addVariable(distributionList);
	addVariable(reliable);
	addVariable(byteStream);

	static bool const reportMessages = ConfigSharedNetwork::getReportMessages();
	if (reportMessages)
	{
		Archive::ReadIterator ri = byteStream.get().begin();
		unsigned long cmdCrc;
		Archive::get(ri, cmdCrc);
		NetworkHandler::reportMessage("send.GameClientMessage." + GameNetworkMessage::getCmdName(cmdCrc), byteStream.get().getSize());
	}
}

//-----------------------------------------------------------------------

GameClientMessage::GameClientMessage(Archive::ReadIterator & source) :
	GameNetworkMessage(s_gameClientMessageName),
	distributionList(),
	reliable(false),
	byteStream()
{
	addVariable(distributionList);
	addVariable(reliable);
	addVariable(byteStream);
	unpack(source);

	static bool const reportMessages = ConfigSharedNetwork::getReportMessages();
	if (reportMessages)
	{
		Archive::ReadIterator ri = byteStream.get().begin();
		unsigned long cmdCrc;
		Archive::get(ri, cmdCrc);
		NetworkHandler::reportMessage("send.GameClientMessage." + GameNetworkMessage::getCmdName(cmdCrc), byteStream.get().getSize());
	}
}

//-----------------------------------------------------------------------

GameClientMessage::~GameClientMessage()
{
}

//-----------------------------------------------------------------------

void GameClientMessage::pack(Archive::ByteStream & target) const
{
	AutoByteStream::pack(target);
}

//-----------------------------------------------------------------------

