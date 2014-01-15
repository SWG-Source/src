// ======================================================================
//
// FirstServerGame.h
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_FirstServerGame_H
#define INCLUDED_FirstServerGame_H

// ======================================================================

#include "sharedFoundation/FirstSharedFoundation.h"

#include <string>
#include <vector>
#include <map>

#ifdef WIN32
// need to boost type_info into the std namespace, as the boost libraries expect it.
#include <typeinfo>
namespace std
{
	typedef ::type_info type_info; //lint !e36 !e18 // error 36: (Error -- redefining the storage class of symbol '_STL::type_info' (type vs. using declaration), conflicts with line 47, file D:\work\swg\current\src\external\3rd\library\stlport453\stlport\typeinfo) // error 18: (Error -- Symbol '_STL::type_info' redeclared (void/nonvoid) conflicts with line 47, file D:\work\swg\current\src\external\3rd\library\stlport453\stlport\typeinfo) // @todo: look into this.
};
#endif


//#include "sharedNetworkMessages/GameNetworkMessage.h"
//#include "sharedFoundation/DynamicVariable.h"
//#include "sharedFoundation/NetworkIdArchive.h"
//#include "sharedObject/CachedNetworkId.h"
//#include "sharedObject/Controller.h"
//#include "sharedObject/NetworkIdManager.h"
//#include "sharedObject/Object.h"
//#include "serverGame/GameServer.h"                      // @todo: this should probably not be in the pch
//#include "serverGame/ServerObject.h"
//#include "StringId.h"

// ======================================================================

#endif

