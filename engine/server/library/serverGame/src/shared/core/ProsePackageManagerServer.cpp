//======================================================================
//
// ProsePackageManagerServer.cpp
// copyright (c) 2002 Sony Online Entertainment
//
//======================================================================

#include "serverGame/FirstServerGame.h"
#include "serverGame/ProsePackageManagerServer.h"

#include "serverGame/ServerObject.h"
#include "sharedGame/ProsePackage.h"
#include "sharedGame/ProsePackageParticipant.h"

//======================================================================

void ProsePackageManagerServer::createSimpleProsePackage            (const ServerObject & target, const StringId & stringId, ProsePackage &  pp)
{
	pp.stringId = stringId;
	ProsePackageManagerServer::createSimpleProsePackageParticipant (target, pp.target);
}

//----------------------------------------------------------------------

void ProsePackageManagerServer::createSimpleProsePackageParticipant (const ServerObject & obj, ProsePackageParticipant & ppp)
{
	ppp.id       = obj.getNetworkId ();
	ppp.str      = obj.getAssignedObjectName ();
	if (ppp.str.empty ())
		ppp.stringId = obj.getObjectNameStringId ();
}

//======================================================================
