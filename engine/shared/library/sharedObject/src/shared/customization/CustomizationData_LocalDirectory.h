// PRIVATE

// ======================================================================
//
// CustomizationData_LocalDirectory.h
// Copyright 2002, 2003 Sony Online Entertainment Inc.
// All Rights Reserved.
// 
// ======================================================================

#ifndef INCLUDED_CustomizationData_LocalDirectory_H
#define INCLUDED_CustomizationData_LocalDirectory_H

// ======================================================================

#include "sharedFoundation/MemoryBlockManagerMacros.h"
#include "sharedObject/CustomizationData_Directory.h"

#include <map>

class MemoryBlockManager;

// ======================================================================

class CustomizationData::LocalDirectory: public CustomizationData::Directory
{
	MEMORY_BLOCK_MANAGER_INTERFACE_WITH_INSTALL;

public:

	explicit LocalDirectory(CustomizationData &owner);
	virtual ~LocalDirectory();

	virtual bool resolvePathNameToDirectory(const std::string &pathName, int pathStartIndex, bool traverseRemoteDirectories, bool createLocalMissingDirectories, Directory *&directory, int &directoryEntryStartIndex);
	virtual bool addVariableTakeOwnership(const std::string &fullVariablePathName, int variableNameStartIndex, CustomizationVariable *variable);

	virtual const CustomizationVariable *findConstVariable(const std::string &variableName, int variableNameStartIndex) const;
	virtual CustomizationVariable       *findVariable(const std::string &variableName, int variableNameStartIndex);

	virtual Directory                   *findDirectory(const std::string &directoryPathName, int directoryNameStartIndex);
	virtual void                         deleteDirectory(Directory *childDirectory);

	virtual void                         iterateOverConstVariables(const std::string &parentPathName, ConstIteratorCallback callback, void *context, bool traverseRemoteDirectories) const;
	virtual void                         iterateOverVariables(const std::string &parentPathName, IteratorCallback callback, void *context, bool traverseRemoteDirectories);

	virtual void                         replaceOrAddDirectory(const std::string &directoryPathName, int directoryNameStartIndex, Directory *directory);

	virtual bool                         linksTo(const CustomizationData &customizationData) const;
	virtual void                         deleteLinksTo(const CustomizationData &customizationData);

	virtual bool                         isLocalDirectory() const;
	
private:

	typedef std::map<const CrcLowerString, Directory*>              DirectoryMap;
	typedef std::map<const CrcLowerString, CustomizationVariable*>  CustomizationVariableMap;

private:
	// Disabled.
	LocalDirectory();
	LocalDirectory(const LocalDirectory&);
	LocalDirectory &operator =(const LocalDirectory&);

private:

	CustomizationVariableMap  m_variables;
	DirectoryMap              m_directories;

};

// ======================================================================

#endif
