//======================================================================
//
// PlayerCreationManagerServer.h
// copyright (c) 2002 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_PlayerCreationManagerServer_H
#define INCLUDED_PlayerCreationManagerServer_H

//======================================================================

#include "sharedFoundation/StationId.h"
#include "sharedGame/PlayerCreationManager.h"
#include "swgSharedUtility/Attributes.def"

class CreatureObject;
class Iff;
class NetworkId;

class PlayerCreationManagerServer : public PlayerCreationManager
{
public:
	static void                   install ();
	static void                   remove ();

	static bool                   setupPlayer(CreatureObject & obj, const std::string & profession, StationId account, bool isJedi);
	static void                   renamePlayer(int8 renameCharacterMessageSource, uint32 stationId, const NetworkId & oid, const Unicode::String & newName, const Unicode::String & oldName, const NetworkId &requestedBy);
};

//======================================================================

#endif
