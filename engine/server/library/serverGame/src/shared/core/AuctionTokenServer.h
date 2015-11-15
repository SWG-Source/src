//======================================================================
//
// AuctionTokenServer.h
// copyright (c) 2002 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_AuctionTokenServer_H
#define INCLUDED_AuctionTokenServer_H

//======================================================================

class AuctionToken;
class ServerObject;

//----------------------------------------------------------------------

class AuctionTokenServer
{
public:
	static AuctionToken createTokenFor         (const ServerObject & obj);
	static void         packTokenAndAttribsOob (const ServerObject & obj, Unicode::String & oob);

};
//======================================================================

#endif
