// CSToolConnection.h
// copyright 2005 Sony Online Entertainment
// Author: Robert Hanz

#ifndef _CSToolConnection_h
#define _CSToolConnection_h

#include "SessionApiClient.h"
#include "sharedFoundation/StationId.h"
#include "sharedNetwork/Connection.h"

#include <set>


class CSToolConnection : public Connection
{
public:
	CSToolConnection(UdpConnectionMT *, TcpClient *);

	CSToolConnection(
		const std::string & address,
		const unsigned short port);

	virtual ~CSToolConnection();

	void                onReceive(const Archive::ByteStream & message);
	void                sendToTool(const std::string & message );
	virtual void	    onConnectionOpened();
	virtual void        onConnectionClosed();
	void		    sendToClusters( const std::string & sCommand, const std::string &sCommandLine );

	static CSToolConnection * getCSToolConnectionByToolId( uint32 id );
	static void validateCSTool( uint32 toolId, apiResult result, const apiSession & session );

	// cluster selection interface
	std::string selectCluster( const std::string & cluster );
	std::string deselectCluster( const std::string & cluster );

protected:

	void		parse( const std::string command );

	uint32 		m_ui32StationID; // 0 = not authenticated
	std::string 	m_sAccountLogin;
	bool 		m_bSecure;
	bool 		m_bLoggedIn;
	uint32		m_ui32PrivilegeLevel;
	std::string	m_sInputBuffer;
	std::string 	m_sUserName;
	uint32	        m_toolID;
	static uint32	m_curToolID;
	std::set< std::string > m_selectedClusters;

private:
	CSToolConnection(const CSToolConnection& );
	CSToolConnection& operator=(const CSToolConnection& );
};

#endif //_CSToolConnection_h
