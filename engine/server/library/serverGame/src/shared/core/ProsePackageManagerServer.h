//======================================================================
//
// ProsePackageManagerServer.h
// copyright (c) 2002 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_ProsePackageManagerServer_H
#define INCLUDED_ProsePackageManagerServer_H

//======================================================================

class ProsePackage;
class ProsePackageParticipant;
class ServerObject;
class StringId;

//----------------------------------------------------------------------

class ProsePackageManagerServer
{
public:

	static void        createSimpleProsePackage            (const ServerObject & target, const StringId & stringId, ProsePackage &            pp);
	static void        createSimpleProsePackageParticipant (const ServerObject & obj, ProsePackageParticipant & pp);
};

//======================================================================

#endif
