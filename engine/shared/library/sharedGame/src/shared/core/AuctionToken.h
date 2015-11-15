//======================================================================
//
// AuctionToken.h
// copyright (c) 2002 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_AuctionToken_H
#define INCLUDED_AuctionToken_H

//======================================================================

#include <string>

//----------------------------------------------------------------------

class AuctionToken
{
public:

	AuctionToken ();

	std::string   sharedTemplateName;
	std::string   customizationData;
};

//======================================================================

#endif
