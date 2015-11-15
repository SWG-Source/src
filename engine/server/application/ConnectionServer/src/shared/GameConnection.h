// GameConnection.h
// copyright 2000 Verant Interactive
// Author: Justin Randall

#ifndef	_GameConnection_H
#define	_GameConnection_H

//-----------------------------------------------------------------------

#include "serverUtility/ServerConnection.h"

class GameCommandChannel;

//-----------------------------------------------------------------------

class GameConnection : public ServerConnection
{
public:
	                     GameConnection(const std::string & remoteAddress, const unsigned short port);
	                     GameConnection(UdpConnectionMT *, TcpClient *);
	virtual              ~GameConnection();

	uint32               getGameServerId() const;
	const std::string &  getSceneName() const;

	void                 setGameServerId(uint32 id);
	void                 setSceneName(const std::string & name);
                
	void                 onConnectionClosed  ();
	void                 onConnectionOpened  ();
	void                 onReceive           (const Archive::ByteStream & message);

private:
	GameConnection();
	GameConnection(const GameConnection&);
	GameConnection& operator=(const GameConnection&);

	uint32                        gameServerId; ///< The DB assigned Id of the connecting gameserver.
	std::string                   sceneName;
};


//-----------------------------------------------------------------------

inline uint32 GameConnection::getGameServerId() const
{
    return gameServerId;
}

//-----------------------------------------------------------------------

inline const std::string & GameConnection::getSceneName() const
{
    return sceneName;
}

//-----------------------------------------------------------------------

inline void GameConnection::setGameServerId(uint32 id)
{
    gameServerId = id;
}

//-----------------------------------------------------------------------

inline void GameConnection::setSceneName(const std::string & name)
{
    sceneName = name;
}

//-----------------------------------------------------------------------




#endif	// _GameConnection_H
