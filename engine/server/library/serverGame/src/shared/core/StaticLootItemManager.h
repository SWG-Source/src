// ======================================================================
//
// StaticLootItemManager.h
//
// Copyright 2004 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_StaticLootItemManager_H
#define INCLUDED_StaticLootItemManager_H

// ======================================================================

class NetworkId;

// ======================================================================

class StaticLootItemManager
{
public:
	typedef stdvector<std::pair<std::string, Unicode::String> >::fwd AttributeVector;

	static void sendDataToClient(NetworkId const & playerId, std::string const & staticItemName);
	static void getAttributes(NetworkId const & playerId, std::string const & staticItemName, AttributeVector & data);
};

// ======================================================================

#endif // INCLUDED_StaticLootItemManager_H

