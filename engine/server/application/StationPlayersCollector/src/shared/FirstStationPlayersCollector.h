// ======================================================================
//
// FirstStationPlayersCollector.h
// copyright (c) 2005 Sony Online Entertainment
// Author: Doug Mellencamp
//
// ======================================================================

#ifndef INCLUDED_FirstStationPlayersCollector_H
#define INCLUDED_FirstStationPlayersCollector_H

// ======================================================================

#include "sharedFoundation/FirstSharedFoundation.h"
#include "sharedNetworkMessages/GameNetworkMessage.h"

#include <string>
#include <vector>
#include <map>

#ifdef WIN32
// need to boost type_info into the std namespace, as the boost libraries expect it.
#include <typeinfo>
namespace std
{
	typedef ::type_info type_info; 
};
#endif

// ======================================================================

#endif

