// PRIVATE

// ======================================================================
//
// CustomizationData_RemoteDirectory.h
// Copyright 2002 Sony Online Entertainment Inc.
// All Rights Reserved.
// 
// ======================================================================

#ifndef INCLUDED_CustomizationData_RemoteDirectory_H
#define INCLUDED_CustomizationData_RemoteDirectory_H

// ======================================================================

#include "sharedFoundation/MemoryBlockManagerMacros.h"
#include "sharedObject/CustomizationData_Directory.h"

class MemoryBlockManager;

// ======================================================================

class CustomizationData::RemoteDirectory: public CustomizationData::Directory
{
	MEMORY_BLOCK_MANAGER_INTERFACE_WITH_INSTALL;

public:

	RemoteDirectory(CustomizationData &remoteCustomizationData, CustomizationData &localCustomizationData, Directory &targetDirectory);
	virtual ~RemoteDirectory();

	virtual bool resolvePathNameToDirectory(const std::string &pathName, int pathStartIndex, bool traverseRemoteDirectories, bool createLocalMissingDirectories, Directory *&directory, int &directoryEntryStartIndex);
	virtual bool addVariableTakeOwnership(const std::string &fullVariablePathName, int variableNameStartIndex, CustomizationVariable *variable);

	virtual const CustomizationVariable *findConstVariable(const std::string &variablePathName, int variableNameStartIndex) const;
	virtual CustomizationVariable       *findVariable(const std::string &variablePathName, int variableNameStartIndex);

	virtual Directory                   *findDirectory(const std::string &directoryPathName, int directoryNameStartIndex);
	virtual void                         deleteDirectory(Directory *childDirectory);

	virtual void                         iterateOverConstVariables(const std::string &parentPathName, ConstIteratorCallback callback, void *context, bool traverseRemoteDirectories) const;
	virtual void                         iterateOverVariables(const std::string &parentPathName, IteratorCallback callback, void *context, bool traverseRemoteDirectories);

	virtual void                         replaceOrAddDirectory(const std::string &directoryPathName, int directoryNameStartIndex, Directory *directory);

	virtual bool                         linksTo(const CustomizationData &customizationData) const;
	virtual void                         deleteLinksTo(const CustomizationData &customizationData);

	virtual bool                         isLocalDirectory() const;
	
private:

	// disabled
	RemoteDirectory();
	RemoteDirectory(const RemoteDirectory&);
	RemoteDirectory &operator =(const RemoteDirectory&);

private:

	CustomizationData &m_remoteCustomizationData;
	Directory         &m_targetDirectory;
	
};

// ======================================================================

#endif
