// ConnectionServerConnection.h
// copyright 2001 Verant Interactive

#ifndef	_ConnectionServerConnection_H
#define	_ConnectionServerConnection_H

//-----------------------------------------------------------------------

#include "serverUtility/ServerConnection.h"

class ConnectionServerCommandChannel;

//-----------------------------------------------------------------------

class ConnectionServerConnection : public ServerConnection
{
public:

	          ConnectionServerConnection   (const std::string & remoteAddress, const uint16 remotePort);
	explicit  ConnectionServerConnection   (UdpConnectionMT *, TcpClient *);
	          ~ConnectionServerConnection  ();

	const uint16          getChatServicePort           () const;
	const uint16          getVoiceChatServicePort      () const;
	const uint16          getCustomerServicePort       () const;
	const uint16          getClientServicePortPrivate  () const;
	const uint16          getClientServicePortPublic   () const;
	const uint16          getGameServicePort           () const;
	int                   getId() const;
	const uint16          getPingPort                  () const;
	int                   getConnectionServerNumber    () const;

	const std::string &   getGameServiceAddress        () const;
	const std::string &   getClientServiceAddress      () const;
	const std::string &   getChatServiceAddress        () const;
	const std::string &   getVoiceChatServiceAddress   () const;
	const std::string &   getCustomerServiceAddress    () const;

	void                  onConnectionClosed           ();
	void                  onConnectionOpened           ();
	void                  onReceive                    (const Archive::ByteStream & message);

	void                  setPlayerCount               (int count);
	int                   getPlayerCount               () const;
	void                  setFreeTrialCount            (int count);
	int                   getFreeTrialCount            () const;
	void                  setEmptySceneCount           (int count);
	int                   getEmptySceneCount           () const;
	void                  setTutorialSceneCount        (int count);
	int                   getTutorialSceneCount        () const;
	void                  setFalconSceneCount          (int count);
	int                   getFalconSceneCount          () const;

	static bool           sendToPseudoClientConnection (unsigned int stationId, const GameNetworkMessage & message);
	static ConnectionServerConnection *  getConnectionForAccount(unsigned int stationId);
	static void           removeFromAccountConnectionMap(unsigned int stationId);

private:
	static int ms_connectionServerNumber;

	uint16          m_chatServicePort;
	uint16          m_voiceChatServicePort;
	uint16          m_csServicePort;
	uint16          m_clientServicePortPrivate;
	uint16          m_clientServicePortPublic;
	uint16          m_gameServicePort;
	int             m_id;
	uint16          m_pingPort;
	int             m_connectionServerNumber;
	std::string     m_gameServiceAddress;
	int             m_playerCount;
	int             m_freeTrialCount;
	int             m_emptySceneCount;
	int             m_tutorialSceneCount;
	int             m_falconSceneCount;
	std::string     m_clientServiceAddress;
	std::string     m_chatServiceAddress;
	std::string     m_voiceChatServiceAddress;
	std::string     m_customerServiceAddress;

	ConnectionServerConnection(const ConnectionServerConnection&);
	ConnectionServerConnection& operator=(const ConnectionServerConnection&);
}; //lint !e1712 default constructor not defined

//-----------------------------------------------------------------------

inline const unsigned short ConnectionServerConnection::getChatServicePort() const
{
	return m_chatServicePort;
}

//-----------------------------------------------------------------------

inline const unsigned short ConnectionServerConnection::getVoiceChatServicePort() const
{
	return m_voiceChatServicePort;
}

//-----------------------------------------------------------------------

inline const unsigned short ConnectionServerConnection::getCustomerServicePort() const
{
	return m_csServicePort;
}

//-----------------------------------------------------------------------

inline const unsigned short ConnectionServerConnection::getClientServicePortPrivate() const
{
	return m_clientServicePortPrivate;
}

//-----------------------------------------------------------------------

inline const unsigned short ConnectionServerConnection::getClientServicePortPublic() const
{
	return m_clientServicePortPublic;
}

//----------------------------------------------------------------------
inline int ConnectionServerConnection::getId() const
{
	return m_id;
}
//----------------------------------------------------------------------

inline const unsigned short ConnectionServerConnection::getGameServicePort() const
{
	return m_gameServicePort;
}

//-----------------------------------------------------------------------

inline const std::string & ConnectionServerConnection::getGameServiceAddress() const
{
	return m_gameServiceAddress;
}

//-----------------------------------------------------------------------

inline const std::string & ConnectionServerConnection::getChatServiceAddress() const
{
	return m_chatServiceAddress;
}

//-----------------------------------------------------------------------

inline const std::string & ConnectionServerConnection::getVoiceChatServiceAddress() const
{
	return m_voiceChatServiceAddress;
}

//-----------------------------------------------------------------------

inline const std::string & ConnectionServerConnection::getCustomerServiceAddress() const
{
	return m_customerServiceAddress;
}

//-----------------------------------------------------------------------

inline void ConnectionServerConnection::setPlayerCount(int count)
{
	m_playerCount = count;
}

//-----------------------------------------------------------------------

inline int ConnectionServerConnection::getPlayerCount() const
{
	return m_playerCount;
}

//----------------------------------------------------------------------

inline void ConnectionServerConnection::setFreeTrialCount(int count)
{
	m_freeTrialCount = count;
}

//-----------------------------------------------------------------------

inline int ConnectionServerConnection::getFreeTrialCount() const
{
	return m_freeTrialCount;
}

//-----------------------------------------------------------------------

inline void ConnectionServerConnection::setEmptySceneCount(int count)
{
	m_emptySceneCount = count;
}

//-----------------------------------------------------------------------

inline int ConnectionServerConnection::getEmptySceneCount() const
{
	return m_emptySceneCount;
}

//-----------------------------------------------------------------------

inline void ConnectionServerConnection::setTutorialSceneCount(int count)
{
	m_tutorialSceneCount = count;
}

//-----------------------------------------------------------------------

inline int ConnectionServerConnection::getTutorialSceneCount() const
{
	return m_tutorialSceneCount;
}

//----------------------------------------------------------------------

inline void ConnectionServerConnection::setFalconSceneCount(int count)
{
	m_falconSceneCount = count;
}

//-----------------------------------------------------------------------

inline int ConnectionServerConnection::getFalconSceneCount() const
{
	return m_falconSceneCount;
}

//----------------------------------------------------------------------

inline const std::string & ConnectionServerConnection::getClientServiceAddress() const
{
	return m_clientServiceAddress;
}

//----------------------------------------------------------------------

inline const uint16 ConnectionServerConnection::getPingPort           () const
{
	return m_pingPort;
}

//----------------------------------------------------------------------

inline int ConnectionServerConnection::getConnectionServerNumber() const
{
	return m_connectionServerNumber;
}

// ======================================================================

#endif	// _ConnectionServerConnection_H
