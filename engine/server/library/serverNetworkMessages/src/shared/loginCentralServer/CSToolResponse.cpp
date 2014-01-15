// CSToolResponse.cpp
// Copyright 2005, Sony Online Entertainment Inc., all rights reserved. 
// Author: Rob Hanz

//-----------------------------------------------------------------------

#include "serverNetworkMessages/FirstServerNetworkMessages.h"
#include "serverNetworkMessages/CSToolRequest.h"
#include "serverNetworkMessages/CSToolResponse.h"

CSToolResponse::CSToolResponse( const CSToolRequest & request, const std::string & sResult) :
GameNetworkMessage( "CSToolResponse" ),
m_accountId( request.getAccountId() ),
m_result( sResult),
m_toolId( request.getToolId() )
{
	addVariable( m_accountId );
	addVariable( m_result );
	addVariable( m_toolId );
}
CSToolResponse::CSToolResponse ( uint32 accountId, const std::string & response, uint32 toolId ) :
GameNetworkMessage( "CSToolResponse" ),
m_accountId( accountId ),
m_result( response ),
m_toolId( toolId )
{
	addVariable( m_accountId );
	addVariable( m_result );
	addVariable( m_toolId );
}

CSToolResponse::CSToolResponse( Archive::ReadIterator & source ) :
GameNetworkMessage("CSToolResponse"),
m_accountId( 0 ),
m_result( "" ),
m_toolId( 0 )
{
	addVariable( m_accountId );
	addVariable( m_result );
	addVariable( m_toolId );
	AutoByteStream::unpack( source );
}

CSToolResponse::~CSToolResponse()
{
}
