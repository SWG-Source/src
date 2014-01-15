// CSToolRequest.cpp
// Copyright 2005, Sony Online Entertainment Inc., all rights reserved. 
// Author: Rob Hanz

//-----------------------------------------------------------------------

#include "serverNetworkMessages/FirstServerNetworkMessages.h"
#include "serverNetworkMessages/CSDBNetMessages.h"


CSGetCharactersRequestMessage::CSGetCharactersRequestMessage (uint32 accountId, uint32 accessLevel, uint32 targetId, uint32 toolId, uint32 loginServer ) :
GameNetworkMessage( "CSGetCharactersRequestMessage" ),
m_accountId( accountId ),
m_targetAccount( targetId ),
m_accessLevel( accessLevel ),
m_toolId( toolId ),
m_loginServerId( loginServer )
{
	addVariable( m_accountId );
	addVariable( m_targetAccount );
	addVariable( m_accessLevel );
	addVariable( m_toolId );
	addVariable( m_loginServerId );
}

CSGetCharactersRequestMessage::CSGetCharactersRequestMessage( Archive::ReadIterator & source ) :
GameNetworkMessage("CSGetCharactersRequestMessage"),
m_accountId(0),
m_targetAccount(0),
m_accessLevel(0),
m_toolId(0),
m_loginServerId(0)
{
	addVariable( m_accountId );
	addVariable( m_targetAccount );
	addVariable( m_accessLevel );
	addVariable( m_toolId );
	addVariable( m_loginServerId );
	AutoByteStream::unpack( source );
}

CSGetCharactersRequestMessage::~CSGetCharactersRequestMessage()
{
}

GetCharactersForAccountCSReplyMsg::GetCharactersForAccountCSReplyMsg (uint32 accountId, uint32 accessLevel, uint32 targetId, uint32 toolId, uint32 loginServer, const std::string& response ) :
GameNetworkMessage( "GetCharactersForAccountCSReplyMsg" ),
m_accountId( accountId ),
m_targetAccount( targetId ),
m_accessLevel( accessLevel ),
m_toolId( toolId ),
m_loginServerId( loginServer ),
m_responseString( response )
{
	addVariable( m_accountId );
	addVariable( m_targetAccount );
	addVariable( m_accessLevel );
	addVariable( m_toolId );
	addVariable( m_loginServerId );
	addVariable( m_responseString );
}

GetCharactersForAccountCSReplyMsg::GetCharactersForAccountCSReplyMsg( Archive::ReadIterator & source ) :
GameNetworkMessage("GetCharactersForAccountCSReplyMsg"),
m_accountId( 0 ),
m_targetAccount( 0 ),
m_accessLevel( 0 ),
m_toolId( 0 ),
m_loginServerId( 0 ),
m_responseString()
{
	addVariable( m_accountId );
	addVariable( m_targetAccount );
	addVariable( m_accessLevel );
	addVariable( m_toolId );
	addVariable( m_loginServerId );
	addVariable( m_responseString );
	AutoByteStream::unpack( source );
}

GetCharactersForAccountCSReplyMsg::~GetCharactersForAccountCSReplyMsg()
{
}


CSGetDeletedItemsRequestMessage::CSGetDeletedItemsRequestMessage(uint32 accountId, uint32 accessLevel, uint32 targetId, uint32 toolId, uint32 loginServer ) :
GameNetworkMessage( "CSGetDeletedItemsRequestMessage" ),
m_accountId( accountId ),
m_targetAccount( targetId ),
m_accessLevel( accessLevel ),
m_toolId( toolId ),
m_loginServerId( loginServer )
{
	addVariable( m_accountId );
	addVariable( m_targetAccount );
	addVariable( m_accessLevel );
	addVariable( m_toolId );
	addVariable( m_loginServerId );
}

CSGetDeletedItemsRequestMessage::CSGetDeletedItemsRequestMessage( Archive::ReadIterator & source ) :
GameNetworkMessage("CSGetDeletedItemsRequestMessage"),
m_accountId( 0 ),
m_targetAccount( 0 ),
m_accessLevel( 0 ),
m_toolId( 0 ),
m_loginServerId( 0 )
{
	addVariable( m_accountId );
	addVariable( m_targetAccount );
	addVariable( m_accessLevel );
	addVariable( m_toolId );
	addVariable( m_loginServerId );
	AutoByteStream::unpack( source );
}

CSGetDeletedItemsRequestMessage::~CSGetDeletedItemsRequestMessage()
{
}


GetDeletedItemsReplyMessage::GetDeletedItemsReplyMessage (uint32 accountId, uint32 accessLevel, const NetworkId & targetId, uint32 toolId, uint32 loginServer, const std::string& response ) :
GameNetworkMessage( "GetDeletedItemsReplyMessage" ),
m_accountId( accountId ),
m_characterId( targetId ),
m_accessLevel( accessLevel ),
m_toolId( toolId ),
m_loginServerId( loginServer ),
m_responseString( response )
{
	addVariable( m_accountId );
	addVariable( m_characterId );
	addVariable( m_accessLevel );
	addVariable( m_toolId );
	addVariable( m_loginServerId );
	addVariable( m_responseString );
}

GetDeletedItemsReplyMessage::GetDeletedItemsReplyMessage( Archive::ReadIterator & source ) :
GameNetworkMessage("GetDeletedItemsReplyMessage"),
m_accountId( 0 ),
m_characterId(),
m_accessLevel( 0 ),
m_toolId( 0 ),
m_loginServerId( 0 ),
m_responseString( "" )
{
	addVariable( m_accountId );
	addVariable( m_characterId );
	addVariable( m_accessLevel );
	addVariable( m_toolId );
	addVariable( m_loginServerId );
	addVariable( m_responseString );
	AutoByteStream::unpack( source );
}

GetDeletedItemsReplyMessage::~GetDeletedItemsReplyMessage()
{
}

GetCharacterIdReplyMessage::GetCharacterIdReplyMessage (uint32 accountId, const NetworkId & targetId, uint32 toolId, uint32 loginServer, const std::string& characterName ) :
GameNetworkMessage( "GetCharacterIdReplyMessage" ),
m_accountId( accountId ),
m_characterId( targetId ),
m_toolId( toolId ),
m_loginServerId( loginServer ),
m_characterName( characterName )
{
	addVariable( m_accountId );
	addVariable( m_characterId );
	addVariable( m_toolId );
	addVariable( m_loginServerId );
	addVariable( m_characterName );
}

GetCharacterIdReplyMessage::GetCharacterIdReplyMessage( Archive::ReadIterator & source ) :
GameNetworkMessage("GetCharacterIdReplyMessage"),
m_accountId( 0 ),
m_characterId(),
m_toolId( 0 ),
m_loginServerId( 0 ),
m_characterName( "" )
{
	addVariable( m_accountId );
	addVariable( m_characterId );
	addVariable( m_toolId );
	addVariable( m_loginServerId );
	addVariable( m_characterName );
	AutoByteStream::unpack( source );
}

GetCharacterIdReplyMessage::~GetCharacterIdReplyMessage()
{
}

DBCSRequestMessage::DBCSRequestMessage( uint32 accountId, uint32 accessLevel, uint32 toolId, uint32 loginServerId, const std::string & command, const std::string & commandLine ) :
	GameNetworkMessage( "DBCSRequestMessage" ),
	m_accountId( accountId ),
	m_accessLevel( accessLevel ),
	m_toolId( toolId ),
	m_loginServer( loginServerId ),
	m_command( command ),
	m_commandLine( commandLine )
{
	addVariable( m_accountId );
	addVariable( m_accessLevel );
	addVariable( m_toolId );
	addVariable( m_loginServer ),
	addVariable( m_command ),
	addVariable( m_commandLine );
}

DBCSRequestMessage::DBCSRequestMessage( Archive::ReadIterator & source ) :
	GameNetworkMessage( "DBCSRequestMessage" ),
	m_accountId( 0 ),
	m_accessLevel( 0 ),
	m_toolId( 0 ),
	m_loginServer( 0 ),
	m_command( "" ),
	m_commandLine( "" )
{
	addVariable( m_accountId );
	addVariable( m_accessLevel );
	addVariable( m_toolId );
	addVariable( m_loginServer ),
	addVariable( m_command ),
	addVariable( m_commandLine );
	
	AutoByteStream::unpack( source );

}

DBCSRequestMessage::~DBCSRequestMessage()
{
}


