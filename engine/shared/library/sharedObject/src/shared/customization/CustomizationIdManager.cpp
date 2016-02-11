// ======================================================================
//
// CustomizationIdManager.cpp
// Copyright 2003 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#include "sharedObject/FirstSharedObject.h"
#include "sharedObject/CustomizationIdManager.h"

#include "sharedFile/Iff.h"
#include "sharedFoundation/ExitChain.h"
#include "sharedFoundation/LessPointerComparator.h"
#include "sharedFoundation/PersistentCrcString.h"
#include "sharedFoundation/PointerDeleter.h"
#include "sharedFoundation/TemporaryCrcString.h"

#include <algorithm>
#include <map>
#include <string>
#include <vector>

// ======================================================================

namespace CustomizationIdManagerNamespace
{
	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	void  remove();

	void  load(Iff &iff);
	void  load_0001(Iff &iff);
	
	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	Tag const TAG_CIDM = TAG(C,I,D,M);
	Tag const TAG_DATA = TAG(D,A,T,A);

	int const cs_firstAssignedId = 1;
	
	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	typedef std::vector<CrcString*>                                 StringVector;
	typedef std::map<CrcString const*, int, LessPointerComparator>  StringIntMap;

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	StringVector  s_idToVariableName;
	StringIntMap  s_variableNameToId;
	
	bool  s_installed;
}

using namespace CustomizationIdManagerNamespace;

// ======================================================================
// namespace CustomizationIdManagerNamespace
// ======================================================================

void CustomizationIdManagerNamespace::remove()
{
	DEBUG_FATAL(!s_installed, ("CustomizationIdManager not installed."));
	s_installed = false;
	
	//-- s_idToVariableName owns the strings.  Delete from here, then clean up vector.
	std::for_each(s_idToVariableName.begin(), s_idToVariableName.end(), PointerDeleter());
	StringVector().swap(s_idToVariableName);

	//-- Clear out s_variableNameToId map.
	s_variableNameToId.clear();
}

// ----------------------------------------------------------------------

void CustomizationIdManagerNamespace::load(Iff &iff)
{
	iff.enterForm(TAG_CIDM);
	{
		Tag const version = iff.getCurrentName();
		switch (version)
		{
			case TAG_0001:
				load_0001(iff);
				break;

			default:
			{
				char name[5];
				ConvertTagToString(version, name);
				FATAL(true, ("CustomizationIdManager: unsupported data file version [%s].", name));
			}
		}
	}
	iff.exitForm(TAG_CIDM);
}

// ----------------------------------------------------------------------

void CustomizationIdManagerNamespace::load_0001(Iff &iff)
{
	iff.enterForm(TAG_0001);
	{
		iff.enterChunk(TAG_DATA);
		{
			for (int expectedId = 1; iff.getChunkLengthLeft() > 0; ++expectedId)
			{
				//-- Read id mapping.
				char variableName[MAX_PATH];
				
				int const id = static_cast<int>(iff.read_int16());
				iff.read_string(variableName, sizeof(variableName) - 1);

				//-- Add to id->variableName vector.
				FATAL(id != expectedId, ("CustomizationIdManager load error: non-consecutive ids listed: expecting [%d], found [%d] for [%s].", expectedId, id, variableName));
				FATAL(id >= 128, ("CustomizationIdManager load error: exceeded limit of supported customization variable IDs [%d] for variable [%s].", id, variableName));

				CrcString *crcVariableName = 0;
				s_idToVariableName.push_back(crcVariableName = new PersistentCrcString(variableName, true));

				//-- Add to variableName->id map.
				std::pair<StringIntMap::iterator, bool> result = s_variableNameToId.insert(StringIntMap::value_type(crcVariableName, id));
				FATAL(!result.second, ("CustomizationIdManager: failed to insert variable name [%s], id [%d] into map. Most likely duplicate entries for variabe name.", variableName, id));
			}
		}
		iff.exitChunk(TAG_DATA);
	}
	iff.exitForm(TAG_0001);
}

// ======================================================================
// class CustomizationIdManager
// ======================================================================

void CustomizationIdManager::install(char const *dataFilename)
{
	DEBUG_FATAL(s_installed, ("CustomizationIdManager already installed."));

	//-- Load up data.
	Iff iff;
	
	bool const openSuccess = iff.open(dataFilename, true);
	FATAL(!openSuccess, ("CustomizationIdManager: could not load initialization data file [%s], are TreeFile paths set correctly?", dataFilename));

	load(iff);
	
	s_installed = true;
	ExitChain::add(remove, "CustomizationIdManager");
}

// ----------------------------------------------------------------------

bool CustomizationIdManager::mapIdToString(int id, std::string &variableName)
{
	DEBUG_FATAL(!s_installed, ("CustomizationIdManager not installed."));

	//-- Adjust id for first-assigned id offset.
	id -= cs_firstAssignedId;

	//-- Handle out-of-range id.
	if ((id < 0) || (id >= static_cast<int>(s_idToVariableName.size())))
	{
		// Not found.
		return false;
	}

	//-- Set string value.
	variableName = NON_NULL(s_idToVariableName[static_cast<StringVector::size_type>(id)])->getString();
	return true;
}

// ----------------------------------------------------------------------

bool CustomizationIdManager::mapIdToString(int id, char *variableName, int bufferLength)
{
	DEBUG_FATAL(!s_installed, ("CustomizationIdManager not installed."));

	//-- Adjust id for first-assigned id offset.
	id -= cs_firstAssignedId;

	//-- Handle out-of-range id.
	if ((id < 0) || (id >= static_cast<int>(s_idToVariableName.size())))
	{
		// Not found.
		return false;
	}

	//-- Set string value.
	NOT_NULL(variableName);
	DEBUG_FATAL(bufferLength < 1, ("CustomizationIdManager: bufferLength of [%d] too small to hold anything.", bufferLength));

	//-- Copy variable name to user buffer, ensure it gets nullptr terminated.
	strncpy(variableName, NON_NULL(s_idToVariableName[static_cast<StringVector::size_type>(id)])->getString(), static_cast<size_t>(bufferLength - 1));
	variableName[bufferLength - 1] = '\0';
	
	return true;
}

// ----------------------------------------------------------------------

bool CustomizationIdManager::mapStringToId(char const *variableName, int &id)
{
	DEBUG_FATAL(!s_installed, ("CustomizationIdManager not installed."));

	//-- Lookup variable name in map.
	TemporaryCrcString const crcVariableName(variableName, true);
	StringIntMap::iterator const findIt = s_variableNameToId.find((const CrcString*)&crcVariableName);
	
	if (findIt == s_variableNameToId.end())
	{
		// Not found.
		return false;
	}
	else
	{
		// Found it.
		id = findIt->second;
		return true;
	}
}

// ======================================================================
