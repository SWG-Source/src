// PRIVATE

// ======================================================================
//
// CustomizationData_Directory.h
// Copyright 2002 Sony Online Entertainment Inc.
// All Rights Reserved.
// 
// ======================================================================

#ifndef INCLUDED_CustomizationData_Directory_H
#define INCLUDED_CustomizationData_Directory_H

// ======================================================================

#include "sharedObject/CustomizationData.h"

// ======================================================================

class CustomizationData::Directory
{
public:

	virtual ~Directory() {}

	virtual bool resolvePathNameToDirectory(const std::string &pathName, int pathStartIndex, bool traverseRemoteDirectories, bool createLocalMissingDirectories, Directory *&directory, int &directoryEntryStartIndex) = 0;
	virtual bool addVariableTakeOwnership(const std::string &fullVariablePathName, int variableNameStartIndex, CustomizationVariable *variable) = 0;

	virtual const CustomizationVariable *findConstVariable(const std::string &variablePathName, int variableNameStartIndex) const = 0;
	virtual CustomizationVariable       *findVariable(const std::string &variablePathName, int variableNameStartIndex) = 0;

	virtual Directory                   *findDirectory(const std::string &directoryPathName, int directoryNameStartIndex) = 0;
	virtual void                         deleteDirectory(Directory *childDirectory) = 0;

	virtual void                         iterateOverConstVariables(const std::string &parentPathName, ConstIteratorCallback callback, void *context, bool traverseRemoteDirectories) const = 0;
	virtual void                         iterateOverVariables(const std::string &parentPathName, IteratorCallback callback, void *context, bool traverseRemoteDirectories) = 0;

	virtual void                         replaceOrAddDirectory(const std::string &directoryPathName, int directoryNameStartIndex, Directory *directory) = 0;

	virtual bool                         linksTo(const CustomizationData &customizationData) const = 0;
	virtual void                         deleteLinksTo(const CustomizationData &customizationData) = 0;

	virtual bool                         isLocalDirectory() const = 0;
	CustomizationData                   &getOwner();
	const CustomizationData             &getOwner() const;

protected:

	explicit Directory(CustomizationData &owner);

private:

	// Disabled.
	Directory();
	Directory(const Directory&);
	Directory &operator =(const Directory&);

private:

	CustomizationData &m_owner;

};

// ======================================================================

inline CustomizationData &CustomizationData::Directory::getOwner()
{
	return m_owner;
}

// ----------------------------------------------------------------------

inline const CustomizationData &CustomizationData::Directory::getOwner() const
{
	return m_owner;
}

// ======================================================================

inline CustomizationData::Directory::Directory(CustomizationData &owner) :
	m_owner(owner)
{
}

// ======================================================================

#endif
