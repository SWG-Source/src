// CentralServerConnection.h
// copyright 2000 Verant Interactive
// Author: Justin Randall

#ifndef	_CentralServerConnection_H
#define	_CentralServerConnection_H

//-----------------------------------------------------------------------

#include "serverUtility/ServerConnection.h"

class ConsoleCommandParserDefault;

//-----------------------------------------------------------------------

class CentralServerConnection : public ServerConnection
{
public:
	CentralServerConnection(const std::string & remoteAddress, const unsigned short remotePort);
	~CentralServerConnection();
	void                          onConnectionClosed      ();
	void                          onConnectionOpened      ();
	const std::string &	          getClusterName          () const;
	void                          onReceive               (const Archive::ByteStream & message);

private:
	CentralServerConnection (const CentralServerConnection&);
	CentralServerConnection& operator= (const CentralServerConnection&);

	ConsoleCommandParserDefault *  m_centralCommandParser;
};

//-----------------------------------------------------------------------

#endif	// _CentralServerConnection_H
