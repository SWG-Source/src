// ======================================================================
//
// PlanetServerConnection.h
// copyright 2000 Verant Interactive
// Author: Justin Randall
// ======================================================================

#ifndef	_PlanetServerConnection_H
#define	_PlanetServerConnection_H

// ======================================================================

#include "serverUtility/ServerConnection.h"
#include <string>

// ======================================================================

class PlanetServerConnection: public ServerConnection
{
public:
	PlanetServerConnection(UdpConnectionMT *, TcpClient *);
	virtual ~PlanetServerConnection();

	virtual void onConnectionOpened();
	virtual void onConnectionClosed();
	virtual void onReceive(Archive::ByteStream const &message);

	void setGameServerConnectionData(std::string const &address, uint16 port);
	std::string const &getGameServerConnectionAddress() const;
	uint16 getGameServerConnectionPort() const;
	std::string const &getSceneId() const;

private:
	PlanetServerConnection(PlanetServerConnection const &);
	PlanetServerConnection &operator=(PlanetServerConnection const &);
	PlanetServerConnection();

	void pushAndClearObjectForwarding();

	std::string m_gameServerConnectionAddress;
	uint16 m_gameServerConnectionPort;
	std::string sceneId;
	std::vector<int> m_forwardCounts;
	std::vector<std::vector<uint32> > m_forwardDestinationServers;
	std::vector<std::pair<Archive::ByteStream, std::vector<uint32> > > m_forwardMessages;
};

// ======================================================================

#endif	// _PlanetServerConnection_H

