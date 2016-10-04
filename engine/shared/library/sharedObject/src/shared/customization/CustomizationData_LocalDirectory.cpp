// ======================================================================
//
// CustomizationData_LocalDirectory.cpp
// Copyright 2002, 2003 Sony Online Entertainment Inc.
// All Rights Reserved.
//
// ======================================================================

#include "sharedObject/FirstSharedObject.h"
#include "sharedObject/CustomizationData_LocalDirectory.h"

#include "sharedFoundation/CrcLowerString.h"
#include "sharedFoundation/ExitChain.h"
#include "sharedFoundation/MemoryBlockManager.h"
#include "sharedFoundation/PointerDeleter.h"
#include "sharedObject/CustomizationVariable.h"
#include "sharedObject/Object.h"

#include <algorithm>
#include <string>
#include <cstdio>

// ======================================================================

MEMORY_BLOCK_MANAGER_IMPLEMENTATION_WITH_INSTALL(CustomizationData::LocalDirectory, true, 0, 0, 0);

// ======================================================================

CustomizationData::LocalDirectory::LocalDirectory(CustomizationData &owner) :
	Directory(owner),
	m_variables(),
	m_directories()
{
}

// ----------------------------------------------------------------------

CustomizationData::LocalDirectory::~LocalDirectory()
{
	//-- delete the variables we own
	std::for_each(m_variables.begin(), m_variables.end(), PointerDeleterPairSecond());

	//-- delete the directories
	std::for_each(m_directories.begin(), m_directories.end(), PointerDeleterPairSecond());
}

// ----------------------------------------------------------------------

bool CustomizationData::LocalDirectory::resolvePathNameToDirectory(const std::string &pathName, int pathStartIndex, bool traverseRemoteDirectories, bool createLocalMissingDirectories, Directory *&directory, int &directoryEntryStartIndex)
{
	//-- check if there's a directory embedded in the path name
	const std::string::size_type endOfDirectoryName   = pathName.find(CustomizationData::cms_directorySeparator, static_cast<std::string::size_type>(pathStartIndex));
	const bool                   hasEmbeddedDirectory = (static_cast<int>(endOfDirectoryName) != static_cast<int>(std::string::npos));

	if (!hasEmbeddedDirectory)
	{
		// The specified pathname does not have another complete subdirectory component
		// embedded within it.  That indicates the given pathName refers to a directory
		// entry that is a direct child of this directory.
		directory                = this;
		directoryEntryStartIndex = pathStartIndex;
		return true;
	}

	//-- handle embedded directory reference

	// get a CrcLowerString for the subdirectory
	const std::string    subdirName = pathName.substr(static_cast<std::string::size_type>(pathStartIndex), endOfDirectoryName - static_cast<std::string::size_type>(pathStartIndex));
	const CrcLowerString crcSubdirName(subdirName.c_str());

	// check if subdir exists
	const DirectoryMap::iterator lowerBoundResult = m_directories.lower_bound(crcSubdirName);
	const bool                   subdirExists     = ((lowerBoundResult != m_directories.end()) && !m_directories.key_comp()(crcSubdirName, lowerBoundResult->first));

	Directory                   *subdir           = 0;

	// handle traversal of directory
	if (subdirExists)
		subdir = lowerBoundResult->second;
	else
	{
		if (!createLocalMissingDirectories)
		{
			// specified subdir doesn't exist and shouldn't be created.  We must bail.
			return false;
		}
		else
		{
			// create the missing subdirectory
			subdir = new LocalDirectory(getOwner());
			IGNORE_RETURN(m_directories.insert(lowerBoundResult, DirectoryMap::value_type(crcSubdirName, subdir)));
		}
	}

	//-- ensure we've got a directory
	if (!subdir)
	{
		WARNING(true, ("resolvePathNameToDirectory(): logic failure, subdir is nullptr"));
		return false;
	}

	//-- traverse the directory
	return subdir->resolvePathNameToDirectory(pathName, static_cast<int>(endOfDirectoryName) + 1, traverseRemoteDirectories, createLocalMissingDirectories, directory, directoryEntryStartIndex);
}

// ----------------------------------------------------------------------

bool CustomizationData::LocalDirectory::addVariableTakeOwnership(const std::string &fullVariablePathName, int variableNameStartIndex, CustomizationVariable *variable)
{
	//-- ensure caller passed in valid customizationVariable
	if (!variable)
	{
		WARNING(true, ("addVariableTakeOwnership(): caller passed in nullptr variable"));
		return false;
	}

	//-- create CrcLowerString name for variable
	const std::string     variableName = fullVariablePathName.substr(static_cast<std::string::size_type>(variableNameStartIndex));
	const CrcLowerString  crcVariableName(variableName.c_str());

	//-- handle assigning variable to variable name
	CustomizationVariableMap::iterator lowerBoundResult = m_variables.lower_bound(crcVariableName);
	const bool variableExists = ((lowerBoundResult != m_variables.end()) && !m_variables.key_comp()(crcVariableName, lowerBoundResult->first));

	if (variableExists)
	{
		//-- replace existing variable.  This probably is in error as the caller can
		//   lookup a value.
		WARNING(true, ("addVariableTakeOwnership(): variable %s already exists, replacing old value", crcVariableName.getString()));
		delete lowerBoundResult->second;
		lowerBoundResult->second = variable;
	}
	else
	{
		//-- insert new variable
		IGNORE_RETURN(m_variables.insert(lowerBoundResult, CustomizationVariableMap::value_type(crcVariableName, variable)));
	}

	// success
	return true;
}

// ----------------------------------------------------------------------

const CustomizationVariable *CustomizationData::LocalDirectory::findConstVariable(const std::string &variablePathName, int variableNameStartIndex) const
{
	const CustomizationVariable *variable = 0;

	{
		//-- create CrcLowerString name for variable
		const std::string     variableName = variablePathName.substr(static_cast<std::string::size_type>(variableNameStartIndex));
		const CrcLowerString  crcVariableName(variableName.c_str());

		//-- check if variable exists
		const CustomizationVariableMap::const_iterator lowerBoundResult = m_variables.lower_bound(crcVariableName);
		const bool variableExists = ((lowerBoundResult != m_variables.end()) && !m_variables.key_comp()(crcVariableName, lowerBoundResult->first));

		if (variableExists)
			variable = lowerBoundResult->second;
	}

	return variable;
}

// ----------------------------------------------------------------------

CustomizationVariable *CustomizationData::LocalDirectory::findVariable(const std::string &variablePathName, int variableNameStartIndex)
{
	CustomizationVariable *variable = 0;

	{
		//-- create CrcLowerString name for variable
		const std::string     variableName = variablePathName.substr(static_cast<std::string::size_type>(variableNameStartIndex));
		const CrcLowerString  crcVariableName(variableName.c_str());

		//-- check if variable exists
		const CustomizationVariableMap::iterator lowerBoundResult = m_variables.lower_bound(crcVariableName);
		const bool variableExists = ((lowerBoundResult != m_variables.end()) && !m_variables.key_comp()(crcVariableName, lowerBoundResult->first));

		if (variableExists)
			variable = lowerBoundResult->second;
	}

	return variable;
}

// ----------------------------------------------------------------------

CustomizationData::Directory *CustomizationData::LocalDirectory::findDirectory(const std::string &directoryPathName, int directoryNameStartIndex)
{
	CustomizationData::Directory *directory = 0;

	{
		//-- create CrcLowerString name for directory
		const std::string     directoryName = directoryPathName.substr(static_cast<std::string::size_type>(directoryNameStartIndex));
		const CrcLowerString  crcDirectoryName(directoryName.c_str());

		//-- check if directory exists
		const DirectoryMap::iterator lowerBoundResult = m_directories.lower_bound(crcDirectoryName);
		const bool directoryExists = ((lowerBoundResult != m_directories.end()) && !m_directories.key_comp()(crcDirectoryName, lowerBoundResult->first));

		if (directoryExists)
			directory = lowerBoundResult->second;
	}

	return directory;
}

// ----------------------------------------------------------------------

void CustomizationData::LocalDirectory::deleteDirectory(Directory *childDirectory)
{
	//-- check for nullptr directory
	if (!childDirectory)
	{
		WARNING(true, ("deleteDirectory(): nullptr childDirectory arg"));
		return;
	}

	//-- find the directory map entry with the given directory
	DirectoryMap::iterator       it    = m_directories.begin();
	const DirectoryMap::iterator endIt = m_directories.end();

	for (; (it != endIt) && (it->second != childDirectory); ++it)
	{
	}

	if (it == m_directories.end())
	{
		WARNING(true, ("deleteDirectory(): specified directory 0x%08x is not a child of this directory", childDirectory));
		return;
	}

	//-- delete child directory
	delete it->second;
	m_directories.erase(it);
}

// ----------------------------------------------------------------------

void CustomizationData::LocalDirectory::iterateOverConstVariables(const std::string &parentPathName, ConstIteratorCallback callback, void *context, bool traverseRemoteDirectories) const
{
	//-- invoke callback for each variable
	{
		const CustomizationVariableMap::const_iterator endIt = m_variables.end();
		for (CustomizationVariableMap::const_iterator it = m_variables.begin(); it != endIt; ++it)
		{
			const std::string fullChildVariablePathName = parentPathName + it->first.getString();
			(*callback)(fullChildVariablePathName, it->second, context);
		}
	}

	//-- invoke iterator over each child directory
	{
		const DirectoryMap::const_iterator endIt = m_directories.end();
		for (DirectoryMap::const_iterator it = m_directories.begin(); it != endIt; ++it)
		{
			//-- check for nullptr directory pointer (shouldn't happen but shouldn't FATAL)
			if (!it->second)
			{
				WARNING(true, ("iterateOverConstVariables(): directory 0x%08x has nullptr child directory for [%s].", this, it->first.getString()));
				return;
			}

			//-- build child directory name
			std::string fullChildDirPathName(parentPathName);
			fullChildDirPathName += it->first.getString();
			fullChildDirPathName += '/';

			//-- instruct child to iterate over its contents
			it->second->iterateOverConstVariables(fullChildDirPathName, callback, context, traverseRemoteDirectories);
		}
	}
}

// ----------------------------------------------------------------------

void CustomizationData::LocalDirectory::iterateOverVariables(const std::string &parentPathName, IteratorCallback callback, void *context, bool traverseRemoteDirectories)
{
	//-- invoke callback for each variable
	{
		const CustomizationVariableMap::iterator endIt = m_variables.end();
		for (CustomizationVariableMap::iterator it = m_variables.begin(); it != endIt; ++it)
		{
			const std::string fullChildVariablePathName = parentPathName + it->first.getString();
			(*callback)(fullChildVariablePathName, it->second, context);
		}
	}

	//-- invoke iterator over each child directory
	{
		const DirectoryMap::iterator endIt = m_directories.end();
		for (DirectoryMap::iterator it = m_directories.begin(); it != endIt; ++it)
		{
			//-- check for nullptr directory pointer (shouldn't happen but shouldn't FATAL)
			if (!it->second)
			{
				WARNING(true, ("iterateOverConstVariables(): directory 0x%08x has nullptr child directory for [%s].", this, it->first.getString()));
				return;
			}

			//-- build child directory name
			std::string fullChildDirPathName(parentPathName);
			fullChildDirPathName += it->first.getString();
			fullChildDirPathName += '/';

			//-- instruct child to iterate over its contents
			it->second->iterateOverVariables(fullChildDirPathName, callback, context, traverseRemoteDirectories);
		}
	}
}

// ----------------------------------------------------------------------

void CustomizationData::LocalDirectory::replaceOrAddDirectory(const std::string &directoryPathName, int directoryNameStartIndex, Directory *directory)
{
	//-- ensure attached directory is not nullptr
	if (!directory)
	{
		WARNING(true, ("replaceOrAddDirectory(): directory arg is nullptr"));
		return;
	}

	//-- create CrcLowerString name for directory
	const std::string     directoryName = directoryPathName.substr(static_cast<std::string::size_type>(directoryNameStartIndex));
	const CrcLowerString  crcDirectoryName(directoryName.c_str());

	//-- check if directory exists
	DirectoryMap::iterator lowerBoundResult = m_directories.lower_bound(crcDirectoryName);
	const bool directoryExists = ((lowerBoundResult != m_directories.end()) && !m_directories.key_comp()(crcDirectoryName, lowerBoundResult->first));

	if (!directoryExists)
	{
		// add new directory map entry
		IGNORE_RETURN(m_directories.insert(lowerBoundResult, DirectoryMap::value_type(crcDirectoryName, directory)));
	}
	else
	{
		// check for self assignment
		if (directory == lowerBoundResult->second)
		{
			WARNING(true, ("replaceOrAddDirectory(): attempting to replace directory [%s] with self", crcDirectoryName.getString()));
			return;
		}

		// delete existing directory
		delete lowerBoundResult->second;

		// replace existing directory with new directory
		lowerBoundResult->second = directory;
	}
}

// ----------------------------------------------------------------------

bool CustomizationData::LocalDirectory::linksTo(const CustomizationData &customizationData) const
{
	UNREF(customizationData);

	//-- LocalDirectory instances never link to remote directories.
	return false;
}

// ----------------------------------------------------------------------

void CustomizationData::LocalDirectory::deleteLinksTo(const CustomizationData &customizationData)
{
	const DirectoryMap::iterator endIt = m_directories.end();
	for (DirectoryMap::iterator it = m_directories.begin(); it != endIt;)
	{
		//-- save old iterator (may be deleted), increment loop iterator.
		DirectoryMap::iterator oldIt = it;
		++it;

		Directory *const directory = oldIt->second;
		if (directory)
		{
			//-- tell the child directory to delete any links it has to the specified customization data.
			directory->deleteLinksTo(customizationData);

			//-- check if child directory links to specified CustomizationData instance.  If so, delete it.
			if (directory->linksTo(customizationData))
			{
				delete oldIt->second;
				m_directories.erase(oldIt);
			}
		}
	}
}

// ----------------------------------------------------------------------

bool CustomizationData::LocalDirectory::isLocalDirectory() const
{
	return true;
}

// ======================================================================
