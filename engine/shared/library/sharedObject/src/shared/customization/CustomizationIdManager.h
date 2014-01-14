// ======================================================================
//
// CustomizationIdManager.h
// Copyright 2003 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#ifndef INCLUDED_CustomizationIdManager_H
#define INCLUDED_CustomizationIdManager_H

// ======================================================================

class CustomizationIdManager
{
public:

	static void install(char const *dataFilename);

	static bool mapIdToString(int id, std::string &variableName);
	static bool mapIdToString(int id, char *variableName, int bufferLength);

	static bool mapStringToId(char const *variableName, int &id);
};

// ======================================================================

#endif
