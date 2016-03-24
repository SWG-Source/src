// ======================================================================
//
// CustomizationData_RemoteDirectory.cpp
// Copyright 2002 Sony Online Entertainment Inc.
// All Rights Reserved.
// 
// ======================================================================

#include "sharedObject/FirstSharedObject.h"
#include "sharedObject/CustomizationData_RemoteDirectory.h"

#include "sharedFoundation/ExitChain.h"
#include "sharedFoundation/MemoryBlockManager.h"

#include <string>

// ======================================================================

MEMORY_BLOCK_MANAGER_IMPLEMENTATION_WITH_INSTALL(CustomizationData::RemoteDirectory, true, 0, 0, 0);

// ======================================================================

CustomizationData::RemoteDirectory::RemoteDirectory(CustomizationData &remoteCustomizationData, CustomizationData &localCustomizationData, Directory &targetDirectory) :
	Directory(localCustomizationData),
	m_remoteCustomizationData(remoteCustomizationData),
	m_targetDirectory(targetDirectory)
{
	//-- tell remote customization data (for which targetDirectory is a local directory)
	//   that the local customization data is dependent on it.
	m_remoteCustomizationData.addDependentCustomizationData(&getOwner());
}

// ----------------------------------------------------------------------

CustomizationData::RemoteDirectory::~RemoteDirectory()
{
	//-- tell remote customization data that the local customization data
	//   no longer is dependent on it.
	m_remoteCustomizationData.removeDependentCustomizationData(&getOwner());
}

// ----------------------------------------------------------------------

bool CustomizationData::RemoteDirectory::resolvePathNameToDirectory(const std::string &pathName, int pathStartIndex, bool traverseRemoteDirectories, bool createLocalMissingDirectories, Directory *&directory, int &directoryEntryStartIndex)
{
	UNREF(createLocalMissingDirectories);
	
	//-- forward the call to the remote directory only if forwarding is permitted
	if (!traverseRemoteDirectories)
		return false;
	else
	{
		// continue to allow traversal of remote directories.
		// do not permit creation of missing directories once we cross a remote directory.
		return m_targetDirectory.resolvePathNameToDirectory(pathName, pathStartIndex, traverseRemoteDirectories, false, directory, directoryEntryStartIndex);
	}
}

// ----------------------------------------------------------------------

bool CustomizationData::RemoteDirectory::addVariableTakeOwnership(const std::string &fullVariablePathName, int variableNameStartIndex, CustomizationVariable *variable)
{
	UNREF(variableNameStartIndex);
	UNREF(variable);
	
	WARNING(true, ("addVariableTakeOwnership([%s]): operation makes no sense on RemoteDirectory instances.", fullVariablePathName.c_str()));
	return false;
}

// ----------------------------------------------------------------------

const CustomizationVariable *CustomizationData::RemoteDirectory::findConstVariable(const std::string &variablePathName, int variableNameStartIndex) const
{
	UNREF(variableNameStartIndex);
	
	WARNING(true, ("findConstVariable([%s]): operation makes no sense on RemoteDirectory instances.", variablePathName.c_str()));
	return 0;
}

// ----------------------------------------------------------------------

CustomizationVariable *CustomizationData::RemoteDirectory::findVariable(const std::string &variablePathName, int variableNameStartIndex)
{
	UNREF(variableNameStartIndex);

	WARNING(true, ("findVariable([%s]): operation makes no sense on RemoteDirectory instances.\n", variablePathName.c_str()));
	return 0;
}

// ----------------------------------------------------------------------

CustomizationData::Directory *CustomizationData::RemoteDirectory::findDirectory(const std::string &directoryPathName, int directoryNameStartIndex)
{
	UNREF(directoryNameStartIndex);

	WARNING(true, ("findDirectory([%s]): operation makes no sense on RemoteDirectory instances.\n", directoryPathName.c_str()));
	return 0;
}

// ----------------------------------------------------------------------

void CustomizationData::RemoteDirectory::deleteDirectory(Directory *childDirectory)
{
	UNREF(childDirectory);

	WARNING(true, ("deleteDirectory(): operation makes no sense on RemoteDirectory instances.\n"));
}

// ----------------------------------------------------------------------

void CustomizationData::RemoteDirectory::iterateOverConstVariables(const std::string &parentPathName, ConstIteratorCallback callback, void *context, bool traverseRemoteDirectories) const
{
	if (traverseRemoteDirectories)
	{
		// foward the call.  continue to allow traversal of remote directories.
		m_targetDirectory.iterateOverConstVariables(parentPathName, callback, context, traverseRemoteDirectories);
	}
}

// ----------------------------------------------------------------------

void CustomizationData::RemoteDirectory::iterateOverVariables(const std::string &parentPathName, IteratorCallback callback, void *context, bool traverseRemoteDirectories)
{
	if (traverseRemoteDirectories)
	{
		// foward the call.  continue to allow traversal of remote directories.
		m_targetDirectory.iterateOverVariables(parentPathName, callback, context, traverseRemoteDirectories);
	}
}

// ----------------------------------------------------------------------

void CustomizationData::RemoteDirectory::replaceOrAddDirectory(const std::string &directoryPathName, int directoryNameStartIndex, Directory *directory)
{
	UNREF(directoryNameStartIndex);

	//-- indicate misuse, delete the directory so it doesn't get leaked
	WARNING(true, ("replaceOrAddDirectory([%s]): operation makes no sense on RemoteDirectory instances.\n", directoryPathName.c_str()));
	delete directory;
}

// ----------------------------------------------------------------------

bool CustomizationData::RemoteDirectory::linksTo(const CustomizationData &customizationData) const
{
	//-- this RemoteDirectory instance links to the specified CustomizationData instance.
	return (&m_remoteCustomizationData == &customizationData);
}

// ----------------------------------------------------------------------

void CustomizationData::RemoteDirectory::deleteLinksTo(const CustomizationData &customizationData)
{
	UNREF(customizationData);

	//-- RemoteDirectory doesn't propagate this call across remote boundaries.
	return;
}

// ----------------------------------------------------------------------

bool CustomizationData::RemoteDirectory::isLocalDirectory() const
{
	return false;
}

// ======================================================================
