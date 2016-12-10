// GameServerConnection.h
// copyright 2001 Verant Interactive
// Author: Justin Randall

#ifndef	_GameServerConnection_H
#define	_GameServerConnection_H

//-----------------------------------------------------------------------

#include "serverUtility/ServerConnection.h"

//-----------------------------------------------------------------------

class GameServerConnection : public ServerConnection
{
public:
	          GameServerConnection   (const std::string & remoteAddress, const unsigned short remotePort);
	explicit  GameServerConnection   (UdpConnectionMT *, TcpClient *);
	virtual   ~GameServerConnection  ();

	void                          onConnectionClosed      ();
	void                          onConnectionOpened      ();
	void                          onReceive               (const Archive::ByteStream & message);

	void queueCompletedChunk (int nodeX, int nodeZ);
	void sendChunkCompleteMessage();
	void clearChunkCompleteQueue();

private:
	typedef std::vector<std::pair<int, int> > ChunkCompleteQueueType;
	ChunkCompleteQueueType * const m_chunkCompleteQueue;
	
private:
	GameServerConnection(const GameServerConnection&);
	GameServerConnection& operator=(GameServerConnection&);
};

//-----------------------------------------------------------------------

#endif // _GameServerConnection_H

