// CSToolRequest.cpp
// Copyright 2005, Sony Online Entertainment Inc., all rights reserved. 
// Author: Rob Hanz


//-----------------------------------------------------------------------

#include "serverNetworkMessages/FirstServerNetworkMessages.h"
#include "serverNetworkMessages/GameServerCSRequestMessage.h"
#include "serverNetworkMessages/GameServerCSResponseMessage.h"


GameServerCSResponseMessage::GameServerCSResponseMessage( const GameServerCSRequestMessage & source ) :
GameNetworkMessage( "GameServerCSResponse" ),
m_accountId( source.getAccountId() ),
m_response( "" ),
m_toolId( source.getToolId() ),
m_loginServerID( source.getLoginServerID() )
{
	addVariable( m_accountId );
	addVariable( m_response );
	addVariable( m_toolId );
	addVariable( m_loginServerID );
}

GameServerCSResponseMessage::GameServerCSResponseMessage( Archive::ReadIterator & source ) :
GameNetworkMessage("GameServerCSResponse"),
m_accountId( 0 ),
m_response( "" ),
m_toolId( 0 ),
m_loginServerID( 0 )
{
	addVariable( m_accountId );
	addVariable( m_response );
	addVariable( m_toolId );
	addVariable( m_loginServerID );
	AutoByteStream::unpack( source );
}

GameServerCSResponseMessage::~GameServerCSResponseMessage()
{
}

void GameServerCSResponseMessage::setResponse( const std::string & response )
{
	m_response.set( response );
}
