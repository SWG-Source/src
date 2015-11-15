// CSToolRequest.cpp
// Copyright 2005, Sony Online Entertainment Inc., all rights reserved. 
// Author: Rob Hanz

//-----------------------------------------------------------------------

#include "serverNetworkMessages/FirstServerNetworkMessages.h"
#include "serverNetworkMessages/CSToolRequest.h"


CSToolRequest::CSToolRequest( uint32 accountId, uint32 accessLevel, const std::string & sCommand, const std::string & sCommandName, uint32 toolId, const std::string & sUserName ) :
GameNetworkMessage( "CSToolRequest" ),
m_accountId( accountId ),
m_command( sCommand ),
m_commandName( sCommandName ),
m_accessLevel( accessLevel ),
m_toolId( toolId ),
m_userName( sUserName )
{
	addVariable( m_accountId );
	addVariable( m_command );
	addVariable( m_commandName );
	addVariable( m_accessLevel );
	addVariable( m_toolId );
	addVariable( m_userName );
}

CSToolRequest::CSToolRequest( Archive::ReadIterator & source ) :
GameNetworkMessage("CSToolRequest"),
m_accountId( 0 ),
m_command( "" ),
m_commandName( "" ),
m_accessLevel( 0 ),
m_toolId( 0 ),
m_userName( "" )
{
	addVariable( m_accountId );
	addVariable( m_command );
	addVariable( m_commandName );
	addVariable( m_accessLevel );
	addVariable( m_toolId );
	addVariable( m_userName );
	AutoByteStream::unpack( source );
}

CSToolRequest::~CSToolRequest()
{
}



