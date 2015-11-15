// GameServerCSToolRequest.cpp
// Copyright 2005, Sony Online Entertainment Inc., all rights reserved. 
// Author: Rob Hanz

//-----------------------------------------------------------------------

#include "serverNetworkMessages/FirstServerNetworkMessages.h"
#include "serverNetworkMessages/GameServerCSRequestMessage.h"
#include "serverNetworkMessages/CSToolRequest.h"


GameServerCSRequestMessage::GameServerCSRequestMessage ( uint32 accountId, 
    							 const std::string & command,
				 			 uint32 toolId,
				 			 uint32 accessLevel, 
				 			 uint32 loginServerID,
							 const std::string & userName,
							 const std::string & commandName ) :
GameNetworkMessage( "GameServerCSRequest" ),
m_accountId( accountId ),
m_command( command ),
m_toolId( toolId ),
m_accessLevel( accessLevel ),
m_loginServerID( loginServerID ),
m_userName( userName ),
m_commandName( commandName )
{
	addVariable( m_accountId );
	addVariable( m_command );
	addVariable( m_toolId );
	addVariable( m_accessLevel );
	addVariable( m_loginServerID );
	addVariable( m_userName );
	addVariable( m_commandName );
}

GameServerCSRequestMessage::GameServerCSRequestMessage( Archive::ReadIterator & source ) :
GameNetworkMessage("GameServerCSRequest"),
m_accountId( 0 ),
m_command( "" ),
m_toolId( 0 ),
m_accessLevel( 0 ),
m_loginServerID( 0 ),
m_userName( "" ),
m_commandName( "" )
{
	addVariable( m_accountId );
	addVariable( m_command );
	addVariable( m_toolId );
	addVariable( m_accessLevel );
	addVariable( m_loginServerID );
	addVariable( m_userName );
	addVariable( m_commandName );
	AutoByteStream::unpack( source );
}

GameServerCSRequestMessage::~GameServerCSRequestMessage()
{
}
