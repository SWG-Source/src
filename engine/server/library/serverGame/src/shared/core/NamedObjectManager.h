// ======================================================================
//
// NamedObjectManager.h
//
// Copyright 2004 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_NamedObjectManager_H
#define INCLUDED_NamedObjectManager_H

// ======================================================================

class NetworkId;

// ======================================================================

class NamedObjectManager
{
public:
	static NetworkId const &getNamedObjectId(std::string const &name);
	static void registerNamedObjectId(std::string const &name, NetworkId const &id);
	static void createZoneObjects();
};

// ======================================================================

#endif // INCLUDED_NamedObjectManager_H

