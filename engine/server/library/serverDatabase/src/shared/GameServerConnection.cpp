// GameServerConnection.cpp
// copyright 2001 Verant Interactive
// Author: Justin Randall


//-----------------------------------------------------------------------

#include "serverDatabase/FirstServerDatabase.h"
#include "Archive/ByteStream.h"
#include "serverDatabase/DatabaseProcess.h"
#include "serverDatabase/GameServerConnection.h"
#include "serverNetworkMessages/ChunkCompleteMessage.h"
#include "serverNetworkMessages/GameGameServerMessages.h"
#include "serverUtility/ConfigServerUtility.h"
#include "sharedNetwork/NetworkSetupData.h"

//-----------------------------------------------------------------------

GameServerConnection::GameServerConnection(const std::string & a, const unsigned short p) :
		ServerConnection(a, p, NetworkSetupData()),
		m_chunkCompleteQueue(new ChunkCompleteQueueType)
{
}

//-----------------------------------------------------------------------

GameServerConnection::GameServerConnection(UdpConnectionMT * u, TcpClient * t) :
		ServerConnection(u, t),
		m_chunkCompleteQueue(new ChunkCompleteQueueType)
{
}

//-----------------------------------------------------------------------

GameServerConnection::~GameServerConnection()
{
	delete m_chunkCompleteQueue;
}

//-----------------------------------------------------------------------

void GameServerConnection::onConnectionClosed()
{
	ServerConnection::onConnectionClosed();
	static MessageConnectionCallback m("GameConnectionClosed");
	emitMessage(m);

	DatabaseProcess::getInstance().gameServerGoByeBye(getProcessId());
}

//-----------------------------------------------------------------------

void GameServerConnection::onConnectionOpened()
{
	ServerConnection::onConnectionOpened();
	static MessageConnectionCallback m("GameConnectionOpened");
	emitMessage(m);

	GameGameServerConnect ggsc(DatabaseProcess::getInstance().getProcessId(), true, ConfigServerUtility::getSpawnCookie(), 0);
	send(ggsc,true);
}

//-----------------------------------------------------------------------

void GameServerConnection::onReceive(const Archive::ByteStream & message)
{
	ServerConnection::onReceive(message);
	Archive::ReadIterator ri = message.begin();
	GameNetworkMessage m(ri);
	if (m.isType("GameGameServerConnect"))
	{
		static MessageConnectionCallback m("GameSetProcessId");
		emitMessage(m);
	}
}

//-----------------------------------------------------------------------

void GameServerConnection::queueCompletedChunk (int nodeX, int nodeZ)
{
	m_chunkCompleteQueue->push_back(std::make_pair(nodeX, nodeZ));
}

// ----------------------------------------------------------------------

void GameServerConnection::sendChunkCompleteMessage()
{
	ChunkCompleteMessage msg(*m_chunkCompleteQueue);
	send(msg, true);
	m_chunkCompleteQueue->clear();
}

// ----------------------------------------------------------------------

void GameServerConnection::clearChunkCompleteQueue()
{
	m_chunkCompleteQueue->clear();
}

// ======================================================================
