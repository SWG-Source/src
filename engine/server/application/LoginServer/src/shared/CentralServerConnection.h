// CentralServerConnection.h
// copyright 2000 Verant Interactive
// Author: Justin Randall

#ifndef	_CentralServerConnection_H
#define	_CentralServerConnection_H

//-----------------------------------------------------------------------

#include "serverNetworkMessages/AvatarList.h"
#include "serverUtility/ServerConnection.h"

class CentralServerCommandChannel;
class TransferCharacterData;

//-----------------------------------------------------------------------

class CentralServerConnection : public ServerConnection
{
public:
	CentralServerConnection(UdpConnectionMT *, TcpClient *);
	
	CentralServerConnection(
		const std::string & address,
		const unsigned short port,
		const std::string &clusterName,
		const uint32 clusterId);
		
	~CentralServerConnection();
	
	const std::string & getClusterName(void) const;
	uint32              getClusterId() const;
	const std::string & getNetworkVersion() const;
	void                onConnectionOpened();
	void                onReceive(const Archive::ByteStream & message);
	void                setClusterName(const std::string & newClusterName);
	void                setClusterId(uint32 clusterId);
	void                setNetworkVersion( const std::string &networkVersion );
	static bool         sendCharacterListResponse(unsigned int, const AvatarList &, const TransferCharacterData &);
	static void         sendToCentralServer(const std::string &, const GameNetworkMessage &);
	
private:
	std::string m_clusterName;
	std::string m_networkVersion;
	uint32      m_clusterId;
	
private:
	CentralServerConnection(const CentralServerConnection&);
	CentralServerConnection& operator=(const CentralServerConnection&);
	void         parseCommand       (const std::string & cmd, int trackId);
}; //lint !e1712 // default constructor not defined

//-----------------------------------------------------------------------

inline const std::string & CentralServerConnection::getClusterName(void) const
{
	return m_clusterName;
}

// ----------------------------------------------------------------------

inline uint32 CentralServerConnection::getClusterId() const
{
	return m_clusterId;
}

//-----------------------------------------------------------------------

inline void CentralServerConnection::setNetworkVersion( const std::string &version )
{
	m_networkVersion = version;
}

//-----------------------------------------------------------------------

inline const std::string &CentralServerConnection::getNetworkVersion() const
{
	return m_networkVersion;
}

// ======================================================================

#endif	// _CentralServerConnection_H
