//======================================================================
//
// AuctionTokenServer.cpp
// copyright (c) 2002 Sony Online Entertainment
//
//======================================================================

#include "serverGame/FirstServerGame.h"
#include "serverGame/AuctionTokenServer.h"

#include "serverGame/ServerObject.h"
#include "serverGame/TangibleObject.h"
#include "sharedGame/AuctionToken.h"
#include "sharedGame/OutOfBandPackager.h"

//======================================================================

AuctionToken AuctionTokenServer::createTokenFor         (const ServerObject & obj)
{
	AuctionToken token;
	token.sharedTemplateName = NON_NULL (obj.getClientSharedTemplateName ());
	
	const TangibleObject * const tangible = dynamic_cast<const TangibleObject *>(&obj);
	if (tangible)
		token.customizationData = tangible->getAppearanceData ();

	return token;
}

//----------------------------------------------------------------------

void AuctionTokenServer::packTokenAndAttribsOob (const ServerObject & obj, Unicode::String & oob)
{
	const AuctionToken & token = createTokenFor (obj);
	OutOfBandPackager::pack (token, 0, oob);

	static OutOfBandPackager::AttributeVector attribs;
	attribs.clear ();
	obj.getAttributes (attribs);

	OutOfBandPackager::pack (attribs, 0, oob);
}

//======================================================================
