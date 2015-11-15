// GameServerConnection.h
// copyright 2000 Verant Interactive
// Author: Justin Randall

#ifndef	_GameServerConnection_H
#define	_GameServerConnection_H

//-----------------------------------------------------------------------
#pragma warning(disable : 4100)	// STL unref'd param warning
#include "serverUtility/ServerConnection.h"
#include <string>

class GameServerCommandChannel;

//-----------------------------------------------------------------------

//@todo change references to SceneID from VolumeName.

class GameServerConnection : public ServerConnection
{
public:
	explicit                      GameServerConnection    (UdpConnectionMT *, TcpClient *);
	virtual				          ~GameServerConnection   ();
	void                          onConnectionClosed      ();
	void                          onConnectionOpened      ();
	void                          onReceive               (const Archive::ByteStream & message);
	const std::string &           getClientServiceAddress () const;
	const uint16                  getClientServicePort    () const;
	const std::string &           getGameServiceAddress   () const;
	const uint16                  getGameServicePort      () const;
	const std::string &           getSceneId              () const;
	bool                          getReady              () const;

private:
	void				          setClientServiceAddress (const std::string & newAddress);
	void				          setClientServicePort    (const uint16 newPort);
	void				          setGameServiceAddress   (const std::string & newAddress);
	void				          setGameServicePort      (const uint16 newPort);
	void				          setSceneId              (const std::string & newVolumeName);

private:
	std::string			          clientServiceAddress;
	uint16				          clientServicePort;
	std::string			          gameServiceAddress;
	uint16				          gameServicePort;
	std::string			          m_sceneId;
	bool                      m_ready;

private:
	GameServerConnection(const GameServerConnection&);
	GameServerConnection& operator=(const GameServerConnection&);
}; //lint !e1712 default constructor not defined

//-----------------------------------------------------------------------

inline const std::string & GameServerConnection::getClientServiceAddress(void) const
{
	return clientServiceAddress;
}

//-----------------------------------------------------------------------

inline const uint16 GameServerConnection::getClientServicePort(void) const
{
	return clientServicePort;
}

//-----------------------------------------------------------------------

inline const std::string & GameServerConnection::getGameServiceAddress(void) const
{
	return gameServiceAddress;
}

//-----------------------------------------------------------------------

inline const uint16 GameServerConnection::getGameServicePort(void) const
{
	return gameServicePort;
}

//-----------------------------------------------------------------------

inline const std::string & GameServerConnection::getSceneId(void) const
{
	return m_sceneId;
}

//-----------------------------------------------------------------------

inline bool GameServerConnection::getReady(void) const
{
	return m_ready;
}

//-----------------------------------------------------------------------

inline void GameServerConnection::setClientServiceAddress(const std::string & newAddress)
{
	clientServiceAddress = newAddress;
}

//-----------------------------------------------------------------------

inline void GameServerConnection::setClientServicePort(const uint16 newPort)
{
	clientServicePort = newPort;
}

//-----------------------------------------------------------------------

inline void GameServerConnection::setGameServiceAddress(const std::string & newAddress)
{
	gameServiceAddress = newAddress;
}

//-----------------------------------------------------------------------

inline void GameServerConnection::setGameServicePort(const uint16 newPort)
{
	gameServicePort = newPort;
}

//-----------------------------------------------------------------------

inline void GameServerConnection::setSceneId(const std::string & newSceneId)
{
	m_sceneId = newSceneId;
}


//-----------------------------------------------------------------------

#endif	// _GameServerConnection_H
