// ======================================================================
//
// GameServerConnection.h
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_GameServerConnection_H
#define INCLUDED_GameServerConnection_H

// ======================================================================

#include "serverUtility/ServerConnection.h"
#include "sharedFoundation/NetworkId.h"

// ======================================================================

class GameServerConnection: public ServerConnection
{
public:
	GameServerConnection(UdpConnectionMT *, TcpClient *);
	virtual ~GameServerConnection();
	virtual void onConnectionClosed();
	virtual void onConnectionOpened();
	virtual void onReceive(Archive::ByteStream const &message);

	void setPreloadNumber(int preloadNumber);
	int getPreloadNumber() const;

private:
	GameServerConnection(GameServerConnection const &);
	GameServerConnection &operator=(GameServerConnection const &);
	GameServerConnection();

	void pushAndClearObjectForwarding();

	int m_preloadNumber;
	std::vector<int> m_forwardCounts;
	std::vector<std::vector<uint32> > m_forwardDestinationServers;
	std::vector<std::pair<Archive::ByteStream, std::vector<uint32> > > m_forwardMessages;
};

//-----------------------------------------------------------------------

inline void GameServerConnection::setPreloadNumber(int preloadNumber)
{
	m_preloadNumber = preloadNumber;
}

//-----------------------------------------------------------------------

inline int GameServerConnection::getPreloadNumber() const
{
	return m_preloadNumber;
}

// ======================================================================

#endif
