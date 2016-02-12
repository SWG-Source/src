// ======================================================================
//
// CustomizationData.cpp
// Copyright 2002 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#include "sharedObject/FirstSharedObject.h"
#include "sharedObject/CustomizationData.h"

#include "sharedFoundation/ExitChain.h"
#include "sharedFoundation/MemoryBlockManager.h"
#include "sharedLog/Log.h"
#include "sharedObject/Appearance.h"
#include "sharedObject/ConfigSharedObject.h"
#include "sharedObject/CustomizationData_Directory.h"
#include "sharedObject/CustomizationData_LocalDirectory.h"
#include "sharedObject/CustomizationData_RemoteDirectory.h"
#include "sharedObject/CustomizationIdManager.h"
#include "sharedObject/CustomizationVariable.h"
#include "sharedObject/Object.h"
#include "UnicodeUtils.h"

#include <algorithm>
#include <map>
#include <string>
#include <vector>
#include <cstdio>

// ======================================================================

MEMORY_BLOCK_MANAGER_IMPLEMENTATION_WITHOUT_INSTALL(CustomizationData, true, 0, 0, 0);

// ======================================================================

const char CustomizationData::cms_directorySeparator   = '/';
const char CustomizationData::cms_stringFieldSeparator = '#';

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

bool CustomizationData::ms_installed;

// ======================================================================

struct CustomizationData::ModificationCallbackData
{
public:

	ModificationCallbackData(ModificationCallback callback, const void *context);

	bool operator ==(const ModificationCallbackData &rhs) const;

public:

	ModificationCallback  m_callback;
	const void           *m_context;

private:

	// disabled
	ModificationCallbackData();
};

// ======================================================================

namespace CustomizationDataNamespace
{
	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	typedef std::vector<std::pair<std::string, CustomizationVariable const*> >  CustomizationVariableConstVector;

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	void  collectPersistedVariablesCallback(std::string const &fullVariablePathName, CustomizationVariable const *customizationVariable, void *context);
	void  alterVariableCallback(std::string const &fullVariablePathName, CustomizationVariable *customizationVariable, void *context);
}

using namespace CustomizationDataNamespace;

// ======================================================================
// namespace CustomizationDataNamespace
// ======================================================================

void CustomizationDataNamespace::collectPersistedVariablesCallback(std::string const &fullVariablePathName, CustomizationVariable const *customizationVariable, void *context)
{
	NOT_NULL(customizationVariable);
	NOT_NULL(context);

	//-- Collect all customization variables that are persistable.
	if (customizationVariable->doesVariablePersist())
	{
		CustomizationVariableConstVector *const variablesToWrite = reinterpret_cast<CustomizationVariableConstVector*>(context);
		variablesToWrite->push_back(CustomizationVariableConstVector::value_type(fullVariablePathName, customizationVariable));
	}
}

// ----------------------------------------------------------------------

void CustomizationDataNamespace::alterVariableCallback(std::string const &fullVariablePathName, CustomizationVariable *customizationVariable, void *context)
{
	UNREF(fullVariablePathName);

	NOT_NULL(customizationVariable);
	NOT_NULL(context);

	CustomizationData *const customizationData = static_cast<CustomizationData*>(context);
	customizationVariable->alter(*customizationData);
}

// ======================================================================
// struct CustomizationData::ModificationCallbackData
// ======================================================================

CustomizationData::ModificationCallbackData::ModificationCallbackData(ModificationCallback callback, const void *context)
:	m_callback(callback),
	m_context(context)
{
}

// ----------------------------------------------------------------------

bool CustomizationData::ModificationCallbackData::operator ==(const ModificationCallbackData &rhs) const
{
	return (rhs.m_callback == m_callback) && (rhs.m_context == m_context);
}

// ======================================================================
// class CustomizationData
// ======================================================================

void CustomizationData::install()
{
	DEBUG_FATAL(ms_installed, ("CustomizationData already installed."));

	installMemoryBlockManager();

	LocalDirectory::install();
	RemoteDirectory::install();

	ms_installed = true;
	ExitChain::add(remove, "CustomizationData");
}

// ----------------------------------------------------------------------

/**
 * Parse an integer value from a string, starting from the specified
 * position and returning the new read position.
 *
 * The string version of the integer is assumed to be in hexadecimal
 * format, not decimal.  It must end in the field separator character,
 * (the '#' character as of this writing).
 *
 * @param data           the string containing the integer to read.
 * @param startPosition  the 0-based index of the first character of
 *                       the integer in the string.
 * @param endPosition    returns the new position to read data, one
 *                       position past the separator character ending
 *                       the int.  A value of -1 indicates a malformed
 *                       hex integer.
 *
 * @return  the integer value of the string contents.
 */

int CustomizationData::parseSeparatedHexInt(const std::string &data, int startPosition, int &endPosition)
{
	//-- find end of int portion of string
	endPosition = static_cast<int>(data.find(cms_stringFieldSeparator, static_cast<std::string::size_type>(startPosition)));
	if (endPosition == static_cast<int>(std::string::npos))
		return static_cast<int>(std::string::npos);

	//-- read value.
	std::string intString(data, static_cast<std::string::size_type>(startPosition), static_cast<std::string::size_type>(endPosition));

	int       value      = static_cast<int>(std::string::npos);
	const int scanResult = sscanf(intString.c_str(), "%x", &value);

	if (scanResult != 1)
	{
		// failed to scan hex int
		endPosition = static_cast<int>(std::string::npos);
		value       = static_cast<int>(std::string::npos);
	}
	else
	{
		// scanned hex int.  move pass the separator character.
		++endPosition;
	}

	return value;
}

// ----------------------------------------------------------------------

/**
 * Parse an embedded string value from a string, starting from the specified
 * position and returning the new read position.
 *
 * @param data           the string containing the embedded string to read.
 * @param startPosition  the 0-based index of the first character of
 *                       the embedded string.
 * @param endPosition    returns the new position to read data, one
 *                       position past the separator character ending
 *                       the embedded string.  A value of -1 indicates a failure
 *                       to read the embedded string.
 *
 * @return  the embedded (i.e. field delimited) string.
 */

std::string CustomizationData::parseSeparatedString(const std::string &data, int startPosition, int &endPosition)
{
	//-- find end of int portion of string
	endPosition = static_cast<int>(data.find(cms_stringFieldSeparator, static_cast<std::string::size_type>(startPosition)));
	if (endPosition == static_cast<int>(std::string::npos))
		return std::string("");

	//-- return the value, incrementing the end position past the separator character.
	const std::string::size_type embeddedStringLength = static_cast<std::string::size_type>(endPosition - startPosition);
	++endPosition;

	return std::string(data, static_cast<std::string::size_type>(startPosition), embeddedStringLength);
}

// ----------------------------------------------------------------------
/**
 * Build a return string that contains the byte values in decimal of the
 * given source string.
 *
 * The return value is similar to using the Oracle SELECT DUMP format:
 *
 *   length=<#bytes>: byte1, byte2, byte3, ...
 */

std::string CustomizationData::makeSelectDumpFormat(char const *source)
{
	NOT_NULL(source);

	//-- Get length of source string.
	size_t const sourceLength = strlen(source);

	//-- Reserve reasonable amount of space for destination string.
	std::string  returnString;
	returnString.reserve(5 * sourceLength);

	//-- Build output header.
	char workBuffer[128];
	IGNORE_RETURN(snprintf(workBuffer, sizeof(workBuffer) - 1, "length=%d: ", static_cast<int>(sourceLength)));
	workBuffer[sizeof(workBuffer) - 1] = '\0';

	returnString += workBuffer;

	//-- Append the ASCII-ized ordinal for each character in the string.
	for (size_t i = 0; i < sourceLength; ++i)
	{
		IGNORE_RETURN(snprintf(workBuffer, sizeof(workBuffer) - 1, "%d%s", static_cast<int>(source[i]), (i < sourceLength - 1) ? ", " : ""));
		workBuffer[sizeof(workBuffer) - 1] = '\0';
		returnString += workBuffer;
	}
	
	return returnString;
}

// ======================================================================

CustomizationData::CustomizationData(Object &owner) :
	m_referenceCount(0),
	m_owner(owner),
	m_rootDirectory(0),
	m_dependentCustomizationDataMap(0),
	m_modificationCallbacks(new ModificationCallbackDataVector())
{
	m_rootDirectory = new LocalDirectory(*this);
}

// ----------------------------------------------------------------------

void CustomizationData::addVariableTakeOwnership(const std::string &fullVariablePathName, CustomizationVariable *variable)
{
	//-- check for nullptr variable
	if (!variable)
	{
		WARNING(true, ("addVariableTakeOwnership() called with nullptr variable.\n"));
		return;
	}

	//-- find the directory to which we'll add the given variable
	const bool traverseRemoteDirectories      = false;
	const bool createLocalMissingDirectories  = true;
	int        variableNameStartIndex         = 0;

	Directory *const targetDirectory = findDirectoryFromPathName(fullVariablePathName, traverseRemoteDirectories, createLocalMissingDirectories, variableNameStartIndex);
	if (!targetDirectory)
	{
		WARNING(true, ("addVariableTakeOwnership(): failed to resolve full variable pathname [%s] to local customization directory.\n", fullVariablePathName.c_str()));
		delete variable;
		return;
	}

	//-- add the variable
	const bool addSuccess = targetDirectory->addVariableTakeOwnership(fullVariablePathName, variableNameStartIndex, variable);
	if (!addSuccess)
	{
		WARNING(true, ("addVariableTakeOwnership(): failed to add variable [%s] to directory for [%s].\n", fullVariablePathName.c_str() + variableNameStartIndex, fullVariablePathName.c_str()));
		delete variable;
		return;
	}

	//-- set the variable's owner
	variable->setOwner(this);
}

// ----------------------------------------------------------------------

const CustomizationVariable *CustomizationData::findConstVariable(const std::string &fullVariablePathName) const
{
	//-- find the directory which owns this variable.
	const bool traverseRemoteDirectories = true;
	int        variableNameStartIndex    = 0;

	const Directory *const targetDirectory = findConstDirectoryFromPathName(fullVariablePathName, traverseRemoteDirectories, variableNameStartIndex);
	if (!targetDirectory)
		return 0;

	//-- find the variable
	return targetDirectory->findConstVariable(fullVariablePathName, variableNameStartIndex);
}

// ----------------------------------------------------------------------

CustomizationVariable *CustomizationData::findVariable(const std::string &fullVariablePathName)
{
	//-- find the directory which owns this variable.
	const bool createLocalMissingDirectories = true;
	const bool traverseRemoteDirectories     = true;
	int        variableNameStartIndex        = 0;

	Directory *const targetDirectory = findDirectoryFromPathName(fullVariablePathName, traverseRemoteDirectories, createLocalMissingDirectories, variableNameStartIndex);
	if (!targetDirectory)
		return 0;

	//-- find the variable
	return targetDirectory->findVariable(fullVariablePathName, variableNameStartIndex);
}

// ----------------------------------------------------------------------
/**
 * Iterate over a const version of each CustomizationVariable.
 *
 * This function will call the specified callback, passing in the context
 * and the next CustomizationVariable with each callback invocation.
 *
 * @param callback                this callback function will be invoked once per
 *                                CustomizationVariable.
 * @param context                 this is the context passed to the callback function.
 * @param includeRemoteVariables  if true, remote (mounted) CustomizationData varaibles
 *                                will be iterated over as well as local variables.
 */

void CustomizationData::iterateOverConstVariables(ConstIteratorCallback callback, void *context, bool includeRemoteVariables) const
{
	NOT_NULL(callback);
	m_rootDirectory->iterateOverConstVariables(std::string("/"), callback, context, includeRemoteVariables);
}

// ----------------------------------------------------------------------
/**
 * Iterate over a modifiable version of each CustomizationVariable.
 *
 * This function will call the specified callback, passing in the context
 * and the next CustomizationVariable with each callback invocation.
 *
 * @param callback                this callback function will be invoked once per
 *                                CustomizationVariable.
 * @param context                 this is the context passed to the callback function.
 * @param includeRemoteVariables  if true, remote (mounted) CustomizationData varaibles
 *                                will be iterated over as well as local variables.
 */

void CustomizationData::iterateOverVariables(IteratorCallback callback, void *context, bool includeRemoteVariables)
{
	NOT_NULL(callback);
	m_rootDirectory->iterateOverVariables(std::string("/"), callback, context, includeRemoteVariables);
}

// ----------------------------------------------------------------------

/**
 * Mount a directory tree of a remote CustomizationData instance to show up in
 * this CustomizationData instance at a specified directory.
 *
 * If the local mount point directory already exists, it and its contents will
 * be deleted when the remote directory is mounted.
 *
 * @param localDirectory  the directory name within the local CustomizationData
 *                        where the remote CustomizationData directory should
 *                        appear.  This string should not end in a foward slash.
 *                        Do not try to mount a remote directory as the root
 *                        directory.
 *
 * @return  true if the remote CustomizationData was attached successfully;
 *          false otherwise.
 */

bool CustomizationData::mountRemoteCustomizationData(CustomizationData &customizationDataToAttach, const std::string &remoteDirectoryPath, const std::string &localDirectoryPath, bool nonExistentLeafDirectoryOk)
{
	//-- find the local directory to which we'll be mounting the remote directory as a subdirectory.
	int        subdirectoryNameStartIndex    = 0;
	const bool traverseRemoteDirectories     = false;
	const bool createLocalMissingDirectories = true;

	Directory *const localParentDirectory = findDirectoryFromPathName(localDirectoryPath, traverseRemoteDirectories, createLocalMissingDirectories, subdirectoryNameStartIndex);
	if (!localParentDirectory)
	{
		WARNING(true, ("mountRemoteCustomizationData(): failed to resolve full localDirectoryPath pathname [%s] to local customization directory.\n", localDirectoryPath.c_str()));
		return false;
	}

	//-- get the remote target Directory instance we're mounting.
	int        targetNameStartIndex                = 0;
	const bool targetTraverseRemoteDirectories     = true;
	const bool targetCreateLocalMissingDirectories = false;

	// ensure target directory ends in a forward slash --- we want to resolve the whole thing.
	Directory *targetDirectory;

	const bool hasEndingSlash = (!remoteDirectoryPath.empty() && (remoteDirectoryPath[remoteDirectoryPath.length()-1] == '/'));
	if (hasEndingSlash)
	{
		// use remote directory name as is
		targetDirectory = customizationDataToAttach.findDirectoryFromPathName(remoteDirectoryPath, targetTraverseRemoteDirectories, targetCreateLocalMissingDirectories, targetNameStartIndex);
	}
	else
	{
		// add ending slash to remote directory name
		std::string realTargetDirectoryPath = remoteDirectoryPath;
		realTargetDirectoryPath += '/';

		targetDirectory = customizationDataToAttach.findDirectoryFromPathName(realTargetDirectoryPath, targetTraverseRemoteDirectories, targetCreateLocalMissingDirectories, targetNameStartIndex);
	}

	if (!targetDirectory)
	{
		if (!nonExistentLeafDirectoryOk)
		{
			// WARNING(true, ("mountRemoteCustomizationData([%s]) on [%s]: remote directory [%s] does not exist", myAppearanceTemplateName, attachAppearanceTemplateName, remoteDirectoryPath.c_str()));
			WARNING(true, ("designer bug: /shared_owner/ customization data missing for object template [%s].", customizationDataToAttach.getOwnerObject().getObjectTemplateName()));
#ifdef _DEBUG
			if (ConfigSharedObject::getLogCustomizationDataIssues())
			{
				DEBUG_REPORT_LOG(true, ("CustomizationData dump for object template [%s] (cd client):\n", m_owner.getObjectTemplateName()));
				debugDump();

				DEBUG_REPORT_LOG(true, ("CustomizationData dump for object template [%s] (cd server):\n", customizationDataToAttach.getOwnerObject().getObjectTemplateName()));
				customizationDataToAttach.debugDump();
			}
#endif
		}
		return false;
	}

	//-- create the RemoteDirectory node that points to the target directory
	Directory *const remoteDirectory = new RemoteDirectory(customizationDataToAttach, *this, *targetDirectory);

	//-- perform the attachment operation
	localParentDirectory->replaceOrAddDirectory(localDirectoryPath, subdirectoryNameStartIndex, remoteDirectory);

	//-- signal to this container that a change occurred (or at least, in theory, could have occurred).
	//   When we attach/detach a parent, we change the view of variables available to this CustomizationData.
	signalModified();

	//-- return success
	return true; //lint !e429 // custodial pointer 'remoteDirectory' not freed or returned // ownership transferred
}

// ----------------------------------------------------------------------
/**
 * Remove a previously mounted remote directory from this CustomizationData
 * instance.
 *
 * @param localDirectoryPathName  the full pathname of the local directory
 *                                currently mounted.  Do not include a
 *                                trailing slash (directory separator).
 *
 * @return  true if the remote CustomizationData was detached successfully;
 *          false otherwise.
 */

bool CustomizationData::dismountRemoteCustomizationData(const std::string &localDirectoryPathName, bool nonExistentLeafDirectoryOk)
{
	//-- ensure there isn't an ending slash
	const bool hasEndingSlash = (!localDirectoryPathName.empty() && localDirectoryPathName[localDirectoryPathName.length()-1] == '/');
	if (hasEndingSlash)
	{
		WARNING(true, ("dismountRemoteCustomizationData(): localDirectoryPathName arg [%s] must not have a trailing directory separator]", localDirectoryPathName.c_str()));
		return false;
	}

	//-- find the local directory
	int        subdirectoryNameStartIndex    = 0;
	const bool traverseRemoteDirectories     = false;
	const bool createLocalMissingDirectories = false;

	Directory *const parentDirectory = findDirectoryFromPathName(localDirectoryPathName, traverseRemoteDirectories, createLocalMissingDirectories, subdirectoryNameStartIndex);
	if (!parentDirectory)
	{
		WARNING(true, ("dismountRemoteCustomizationData(): local parent of directory [%s] does not exist", localDirectoryPathName.c_str()));
		return false;
	}

	//-- ensure the directory is a remote directory
	Directory *const baseRemoteDirectory = parentDirectory->findDirectory(localDirectoryPathName, subdirectoryNameStartIndex);
	if (!baseRemoteDirectory)
	{
		WARNING(!nonExistentLeafDirectoryOk, ("dismountRemoteCustomizationData(): leaf directory [%s] does not exist.", localDirectoryPathName.c_str()));
		return false;
	}

	RemoteDirectory *const remoteDirectory = dynamic_cast<RemoteDirectory*>(baseRemoteDirectory);
	if (!remoteDirectory)
	{
		WARNING(true, ("dismountRemoteCustomizationData(): directory [%s] exists but is not a mounted directory.", localDirectoryPathName.c_str()));
		return false;
	}

	//-- delete the remote directory entry from the parent
	parentDirectory->deleteDirectory(remoteDirectory);

	//-- return success
	return true;
}

// ----------------------------------------------------------------------
/**
 * Retrieve the owner CustomizationData instance for the variables in the given 
 * directory.
 *
 * The CustomizationData instance returned will be the CustomizationData instance
 * that persists (or are local to) the variables in the directory.  When the
 * variables are local to this CustomizationData instance, this instance is returned.
 * When the directory is mounted from another CustomizationData instance, the server
 * CustomizationData instance for the mount is returned.
 *
 * This function is useful to wearables that need to traverse to the owner Object.
 * The wearable can traverse to the CustomizationData instance for the /shared_owner
 * directory, and from there can get the owner Object instance.
 *
 * @param directory  the directory containing variables that are persisted by the
 *                   CustomzationData instance returned by this function.  This directory
 *                   should contain a trialing directory separator.
 *
 * @return  the CustomizationData instance that persists the variables in the given
 *          directory.
 */

const CustomizationData *CustomizationData::getDirectoryOwner(const std::string &directory) const
{
	//-- Ensure there is an ending slash.
	const bool hasEndingSlash = (!directory.empty() && directory[directory.length()-1] == '/');
	if (!hasEndingSlash)
	{
		DEBUG_WARNING(true, ("getDirectoryOwner(): directory arg [%s] must have a trailing directory separator].", directory.c_str()));
		return 0;
	}

	//-- Find the directory.
	int        subdirectoryNameStartIndex    = 0;
	const bool traverseRemoteDirectories     = true;

	const Directory *const parentDirectory = findConstDirectoryFromPathName(directory, traverseRemoteDirectories, subdirectoryNameStartIndex);
	if (!parentDirectory)
	{
		DEBUG_WARNING(ConfigSharedObject::getLogCustomizationDataIssues(), ("getDirectoryOwner(): directory [%s] does not exist.", directory.c_str()));
#ifdef _DEBUG
		if (ConfigSharedObject::getLogCustomizationDataIssues())
			debugDump();
#endif
		return 0;
	}

	//-- Return the owner CustomizationData instance for the directory.
	return &(parentDirectory->getOwner());
}

// ----------------------------------------------------------------------

std::string CustomizationData::writeLocalDataToString() const
{
	//-- Save binary version of state into byte buffer.
	ByteVector  binaryData;
	
	saveToByteVector(binaryData);

	//-- Convert binary data to string.  We're escaping the 0 for the database so this is a non-nullptr string.
	//   We translate 0x00 => 0xff 0x01
	//                0xff => 0xff 0x02
	std::string  returnValue;

	returnValue.reserve(static_cast<size_t>(static_cast<float>(binaryData.size()) * 1.25f));
	
	ByteVector::iterator const endIt = binaryData.end();
	for (ByteVector::iterator it = binaryData.begin(); it != endIt; ++it)
	{
		byte const value = *it;
		switch (value)
		{
			case 0:
				returnValue.push_back(static_cast<std::string::value_type>(-1));
				returnValue.push_back(0x01);
				break;

			case 0xff:
				returnValue.push_back(static_cast<std::string::value_type>(-1));
				returnValue.push_back(0x02);
				break;

			default:
				returnValue.push_back(static_cast<std::string::value_type>(value));
		}
	}

	// Append non-space end-of-data escape sequence so database doesn't truncate trailing values that just happen to be whitespace.
	returnValue.push_back(static_cast<std::string::value_type>(-1));
	returnValue.push_back(static_cast<std::string::value_type>(3));

	// We need our string to be a compatible utf8 string
	// so convert our narrow string to a wide string and
	// then to a utf8 string
	return Unicode::wideToUTF8( Unicode::narrowToWide( returnValue ) );
}

// ----------------------------------------------------------------------

void CustomizationData::loadLocalDataFromString(const std::string &stringData)
{
	// The first byte of the data is the version number and fortunately
	// it is also the the first byte of the string
	if (stringData.size() > 0 )
	{
		std::string::value_type const version = stringData[0];
		switch (version)
		{
			case 1:
				loadLocalDataFromString_1(stringData);
				break;

			case 2:
				loadLocalDataFromString_2(stringData);
				break;

			default:
			{
				Object const &object = getOwnerObject();
				WARNING(true, ("object id [%s], template [%s] tried to load local data from string from unsupported version [%d].", object.getNetworkId().getValueString().c_str(), object.getObjectTemplateName(), static_cast<int>(version)));
			}
		}
	}
}

// ----------------------------------------------------------------------

void CustomizationData::loadLocalDataFromString_1(const std::string &stringData)
{
	Object const &object = getOwnerObject();
	
	//-- Convert string to binary ByteVector.
	//   We translate 0xff 0x01 => 0x00
	//                0xff 0x02 => 0xff
	ByteVector  binaryData;

	binaryData.reserve(stringData.size());
	
	std::string::const_iterator const endIt = stringData.end();
	for (std::string::const_iterator it = stringData.begin(); it != endIt; ++it)
	{
		std::string::value_type const character = *it;
		switch (character)
		{
			case static_cast<std::string::value_type const>(-1):
			{
				//-- Handle escaped data.
				std::string::const_iterator nextIt = it + 1;
				if (nextIt == endIt)
				{
					WARNING(true, ("object id [%s], template [%s]: corrupt customization data, unexpected end of data, invalid string form [%s].", object.getNetworkId().getValueString().c_str(), object.getObjectTemplateName(), stringData.c_str()));
					
					std::string const corruptDataDump = CustomizationData::makeSelectDumpFormat(stringData.c_str());
					LOG("customization-data-loss", ("corrupt encoded data (expecting escaped char) for id=[%s],template=[%s], corrupt data in SELECT DUMP format:[%s]", getOwnerObject().getNetworkId().getValueString().c_str(), getOwnerObject().getObjectTemplateName(), corruptDataDump.c_str()));
					return;
				}
				else
				{
					std::string::value_type const nextCharacter = *nextIt;
					switch (nextCharacter)
					{
						case 0x01:
							// This is an escaped 0x00.
							binaryData.push_back(0x00);

							// Consume the character.
							++it;
							break;

						case 0x02:
							// This is an escaped 0xff.
							binaryData.push_back(0xff);

							// Consume the character.
							++it;
							break;

						case 0x03:
							{
								// This marks the end of data.
								std::string::const_iterator finalIt = nextIt + 1;
								WARNING(finalIt != endIt, ("object id [%s], template [%s]: corrupt customization data, found end-of-data escape sequence while not at end of data, invalid string form [%s], ignoring.", object.getNetworkId().getValueString().c_str(), object.getObjectTemplateName(), stringData.c_str()));

								// Consume the character.
								++it;
							}
							break;
							
						default:
							WARNING(true, ("object id [%s], template [%s]: corrupt customization data, unsupported escape character ord=[%d], invalid string form [%s].", object.getNetworkId().getValueString().c_str(), object.getObjectTemplateName(), static_cast<int>(nextCharacter), stringData.c_str()));

							std::string const corruptDataDump = CustomizationData::makeSelectDumpFormat(stringData.c_str());
							LOG("customization-data-loss", ("corrupt encoded data (unsupported escaped char: ord=%d) for id=[%s],template=[%s],corrupt data in SELECT DUMP format:[%s]", static_cast<int>(nextCharacter), getOwnerObject().getNetworkId().getValueString().c_str(), getOwnerObject().getObjectTemplateName(), corruptDataDump.c_str()));
							return;
					}
				}
				break;
			}

			default:
				//-- Handle unescaped data.
				binaryData.push_back(static_cast<byte>(character));
		}
	}

	//-- Restore state from binary data.
	bool const restoreResult = restoreFromByteVector(binaryData);
	WARNING(!restoreResult, ("object id [%s], template [%s]: customization data restore error!", object.getNetworkId().getValueString().c_str(), object.getObjectTemplateName()));
}

// ----------------------------------------------------------------------

void CustomizationData::loadLocalDataFromString_2(const std::string &stringData)
{
	// Version 2 of the data has the string encoded as utf8
	loadLocalDataFromString_1( Unicode::wideToNarrow( Unicode::utf8ToWide( stringData ) ) );
}

// ----------------------------------------------------------------------
/**
 * Registers a callback that will be invoked whenever a modification is
 * made to a variable of this instance.
 *
 * The callback will be invoked whenever a variable local to this
 * CustomizationData instance is modified.  In addition, the callback
 * will be invoked whenever a mounted CustomizationData instance is
 * modified.
 *
 * @param modificationCallback  callback function to invoke after variables
 *                              are modified.
 * @param context               context value that will be passed to the
 *                              callback whenever the callback is invoked.
 *                              This instance will never modify the context.
 */

void CustomizationData::registerModificationListener(ModificationCallback modificationCallback, const void *context)
{
	m_modificationCallbacks->push_back(ModificationCallbackData(modificationCallback, context));
}

// ----------------------------------------------------------------------
/**
 * Remove the registration of a modification callback previously made
 * via registerModificationListener().
 *
 * The caller should use this function to suppress future modification
 * notification callbacks to a given callback/context combination.
 *
 * The caller must provide the identical callback function and context
 * in order to deregister it.
 *
 * @param modificationCallback  callback function to remove.
 * @param context               context associated with callback function
 *                              to remove.
 */

void CustomizationData::deregisterModificationListener(ModificationCallback modificationCallback, const void *context)
{
	//-- find the specified callback
	ModificationCallbackDataVector::iterator findIt = std::find(m_modificationCallbacks->begin(), m_modificationCallbacks->end(), ModificationCallbackData(modificationCallback, context));
	if (findIt == m_modificationCallbacks->end())
	{
		WARNING(true, ("specified callback/context pair not registered [0x%08x/0x%08x]", modificationCallback, context));
		return;
	}

	//-- erase the callback
	IGNORE_RETURN(m_modificationCallbacks->erase(findIt));
}

// ----------------------------------------------------------------------
/**
 * Notifies this CustomizationData instance that a local variable has
 * been modified.
 *
 * Only CustomizationVariable-derived classes should need to call this.
 */

void CustomizationData::signalModified() const
{
	//-- call all modification functions
	{
		const ModificationCallbackDataVector::iterator endIt = m_modificationCallbacks->end();
		for (ModificationCallbackDataVector::iterator it = m_modificationCallbacks->begin(); it != endIt; ++it)
		{
			// invoke the callback
			(*(it->m_callback))(*this, it->m_context);
		}
	}

	//-- propagate modification signal to all CustomizationData instances
	//   that have mounted this CustomizationData instance.
	if (m_dependentCustomizationDataMap)
	{
		const CustomizationDataIntMap::iterator endIt = m_dependentCustomizationDataMap->end();
		for (CustomizationDataIntMap::iterator it = m_dependentCustomizationDataMap->begin(); it != endIt; ++it)
		{
			// signal the dependent CustomizationData
			it->first->signalModified();
		}
	}
}

// ----------------------------------------------------------------------
/**
 * Decrement the reference count associated with the CustomizationData
 * instance.
 *
 * The user should fetch() a reference for each logical reference the
 * user makes to the CustomizationData instance.  When a logical reference
 * no longer is needed, the caller should call release().
 *
 * When the reference count reaches zero, the CustomizationData will
 * be destroyed.
 *
 * @see CustomizationData::release()
 */

void CustomizationData::release() const
{
	--m_referenceCount;
	if (m_referenceCount < 1)
	{
		DEBUG_FATAL(m_referenceCount < 0, ("bad reference counting code, releasing dead object"));
		delete const_cast<CustomizationData*>(this);
	}
}

// ----------------------------------------------------------------------

void CustomizationData::alterLocalVariables()
{
	iterateOverVariables(alterVariableCallback, this, false);
}

// ----------------------------------------------------------------------

#ifdef _DEBUG

/**
 * Log a dump of all variables accessible via this CustomizationData instance.
 */

void CustomizationData::debugDump() const
{
	DEBUG_REPORT_LOG(true, ("BEGIN: CustomizationData debugDump\n"));

	iterateOverConstVariables(debugDumpIterationCallback, 0);

	DEBUG_REPORT_LOG(true, ("END: CustomizationData debugDump\n"));
}

#endif

// ======================================================================

void CustomizationData::remove()
{
	DEBUG_FATAL(!ms_installed, ("CustomizationData not installed."));
	ms_installed = false;

	removeMemoryBlockManager();
}

// ----------------------------------------------------------------------

#ifdef _DEBUG

void CustomizationData::debugDumpIterationCallback(const std::string &fullVariablePathName, const CustomizationVariable *customizationVariable, void *context)
{
	UNREF(context);
	NOT_NULL(customizationVariable);

	DEBUG_REPORT_LOG(true, ("  %s: %s\n", fullVariablePathName.c_str(), customizationVariable->debugToString().c_str()));
}

#endif

// ======================================================================

CustomizationData::~CustomizationData()
{
	//-- tell any CustomizationData instances dependent on this one that this
	//   instance is dying.  Allow those instances to destroy any remote directories
	//   attached to this one.
	if (m_dependentCustomizationDataMap)
	{
		// note: the underlying map will be modified during loop iterations.  I'm not
		// 100% certain in node-based structures if I can count on the end() value staying
		// constant while map entries are deleted.  If this section ever causes problems, remove
		// the const endIt and replace the loop test with a call to the end() function call.
		const CustomizationDataIntMap::iterator endIt = m_dependentCustomizationDataMap->end();
		for (CustomizationDataIntMap::iterator it = m_dependentCustomizationDataMap->begin(); it != endIt;)
		{
			// preserve looping iterator since notifyPendingRemoteDestruction() will destroy its associated iterator.
			CustomizationDataIntMap::iterator oldIt = it;
			++it;

			oldIt->first->notifyPendingRemoteDestruction(this);
		}

		delete m_dependentCustomizationDataMap;
	}

	delete m_modificationCallbacks;
	delete m_rootDirectory;
}

// ----------------------------------------------------------------------
/**
 * Identical to findDirectoryFromPathName(), except you can never create
 * local missing directories.
 *
 * @see CustomizationData::findDirectoryFromPathName()
 */

const CustomizationData::Directory *CustomizationData::findConstDirectoryFromPathName(const std::string &variablePathName, bool traverseRemoteDirectories, int &directoryEntryStartIndex) const
{
	//-- check for zero-length path name
	if (variablePathName.empty())
	{
		WARNING(true, ("findDirectoryFromVariablePath() called with empty variablePathName name."));
		return 0;
	}

	//-- all directories are assumed to be relative to root.  There is no current directory
	//   concept.  CustomizationData accepts both "/subdir1/variable1" and "subdir1/variable1"
	//   as starting at the root directory.
	const int pathStartIndex = (variablePathName[0] == '/') ? 1 : 0;

	//-- find the directory referenced by the path name.
	const bool createMissingLocalDirectories = false;
	Directory *targetDirectory               = 0;

	const bool resolveSuccess = m_rootDirectory->resolvePathNameToDirectory(variablePathName, pathStartIndex, traverseRemoteDirectories, createMissingLocalDirectories, targetDirectory, directoryEntryStartIndex);
	if (resolveSuccess)
		return targetDirectory;
	else
		return 0;
}

// ----------------------------------------------------------------------
/**
 * Find the Directory instance and the start of the path name directory
 * entry from a full path name.
 *
 * Path names are of the form /xxx/yyy/zzz or xxx/yyy/zzz, where
 * xxx and yyy represent a hierarchical directory structure and zzz
 * represents an entry in the "/xxx/yyy/" subdirectory tree.  zzz may
 * be either a variable entry or another subdirectory entry.  The
 * directory structure may be as deep as the caller desires.
 *
 * If the caller wants /xxx/yyy/ returned as the directory, and the start
 * of zzz returned as the directory entry, then zzz must not end in
 * the forward slash.  If it did end in a forward slash, then the
 * directory "/xxx/yyy/zzz" would be returned with the directory entry
 * name starting at the end of the string (i.e. nothing).
 *
 * Intermediate missing local directories can be created as an option.
 * Remote directories will never be created.
 *
 * Optionally the Directory search can suppress traversal of remote
 * links.
 */

CustomizationData::Directory *CustomizationData::findDirectoryFromPathName(const std::string &variablePathName, bool traverseRemoteDirectories, bool createLocalMissingDirectories, int &directoryEntryStartIndex)
{
	//-- check for zero-length path name
	if (variablePathName.empty())
	{
		WARNING(true, ("findDirectoryFromVariablePath() called with empty variablePathName name."));
		return 0;
	}

	//-- all directories are assumed to be relative to root.  There is no current directory
	//   concept.  CustomizationData accepts both "/subdir1/variable1" and "subdir1/variable1"
	//   as starting at the root directory.
	const int pathStartIndex = (variablePathName[0] == '/') ? 1 : 0;

	//-- find the directory referenced by the path name.
	Directory *targetDirectory = 0;

	const bool resolveSuccess = m_rootDirectory->resolvePathNameToDirectory(variablePathName, pathStartIndex, traverseRemoteDirectories, createLocalMissingDirectories, targetDirectory, directoryEntryStartIndex);
	if (resolveSuccess)
		return targetDirectory;
	else
		return 0;
}

// ----------------------------------------------------------------------
/**
 * Indicate the given CustomizationData instance has a dependency on this
 * CustomizationData instance.
 *
 * When another CustomizationData instance has a dependency on this
 * CustomizationData instance, the other instance will receive a
 * signalModified() invocation whenever any variables associated with
 * this instance are modified.  Also, if this CustomizationData instance
 * is destroyed, all dependencent instances will receive a
 * notifyPendingRemoteDestruction() invocation prior to this instance destruction.
 * This hook allows dependent CustomizationData instances to remove
 * remote links to this dying instance.
 *
 * A dependent CustomizationData instance must unhook itself from
 * this instance prior the dependent's destruction via a call to
 * removeDependentCustomizationData.
 *
 * A reference count is associated with each CustomizationData since
 * it is entirely legal for multiple directories on one instance to
 * mount to multiple remote directories on another instance.  The depends-on
 * link is only broken once the reference count is decremented to zero.
 * Each call to addDependentCustomizationData() requires a matching call to
 * removeDependentCustomizationData().
 *
 * @param customizationData  the CustomizationData instance dependent
 *                           on this instance.
 *
 * @see removeDependentCustomizationData()
 * @see notifyPendingRemoteDestruction()
 */

void CustomizationData::addDependentCustomizationData(CustomizationData *customizationData)
{
	//-- ensure the dependent CustomizationData map exists
	if (!m_dependentCustomizationDataMap)
		m_dependentCustomizationDataMap = new CustomizationDataIntMap();

	//-- determine if an entry for the dependent customizationData exists
	CustomizationDataIntMap::iterator lowerBoundResult = m_dependentCustomizationDataMap->lower_bound(customizationData);
	const bool entryExists = ((lowerBoundResult != m_dependentCustomizationDataMap->end()) && !m_dependentCustomizationDataMap->key_comp()(customizationData, lowerBoundResult->first));

	if (!entryExists)
	{
		// create entry, set reference count to 1
		IGNORE_RETURN(m_dependentCustomizationDataMap->insert(lowerBoundResult, CustomizationDataIntMap::value_type(customizationData, 1)));
	}
	else
	{
		// entry exists, bump up reference count.
		++(lowerBoundResult->second);
	}
}

// ----------------------------------------------------------------------
/**
 * Indicate the given CustomizationData instance had a dependency on this
 * CustomizationData instance, but no longer requires it.
 *
 * A dependent CustomizationData instance must unhook itself from
 * this instance prior the dependent's destruction.
 *
 * A reference count is associated with each CustomizationData since
 * it is entirely legal for multiple directories on one instance to
 * mount to multiple remote directories on another instance.  The depends-on
 * link is only broken once the reference count is decremented to zero.
 * Each call to addDependentCustomizationData() requires a matching call to
 * removeDependentCustomizationData().
 *
 * @param customizationData  the CustomizationData instance formerly dependent
 *                           on this instance.
 *
 * @see addDependentCustomizationData()
 * @see notifyPendingRemoteDestruction()
 */

void CustomizationData::removeDependentCustomizationData(CustomizationData *customizationData)
{
	bool hasEntry = false;

	if (m_dependentCustomizationDataMap)
	{
		//-- find the customization data entry
		CustomizationDataIntMap::iterator findIt = m_dependentCustomizationDataMap->find(customizationData);
		if (findIt != m_dependentCustomizationDataMap->end())
		{
			//-- decrement reference count on the specified customizationData.
			--(findIt->second);

			//-- check for removal of entry
			if (findIt->second < 1)
				m_dependentCustomizationDataMap->erase(findIt);

			//-- indicate we found the entry.
			hasEntry = true;
		}
	}

	WARNING(!hasEntry, ("removeDependentCustomizationData(): customizationData [0x%08x] not dependent on this instance.", customizationData));
}

// ----------------------------------------------------------------------
/**
 * Indicates the given CustomizationData instance argument, on which the called
 * instance is dependent, is about to be destroyed.
 *
 * This function allows a CustomizationData instance dependent on another instance
 * to remove any remote links to the about-to-be-destroyed instance.
 *
 * @param customizationData  the instance that is about to die, and which the called
 *                           instance is dependent on.
 */

void CustomizationData::notifyPendingRemoteDestruction(const CustomizationData *customizationData)
{
	//-- validate arg
	if (!customizationData)
	{
		DEBUG_WARNING(true, ("notifyPendingRemoteDestruction(): customizationData arg is nullptr"));
		return;
	}

	//-- delete any directory that links to the specified customization data.
	m_rootDirectory->deleteLinksTo(*customizationData);
}

// ----------------------------------------------------------------------

void CustomizationData::saveToByteVector(ByteVector &data, bool persistRemoteData) const
{
	//-- Collect all persistable customization variables.
	CustomizationVariableConstVector  variables;
	iterateOverConstVariables(collectPersistedVariablesCallback, &variables, persistRemoteData);

	//-- Write header data.
	// version 1.
	data.push_back(2);

	// Write # variables.
	int const variableCount = static_cast<int>(variables.size());
	FATAL(variableCount > 255, ("CustomizationData persistence format must be upgraded.  version 1 only supports 255 persisted variables but object id [%s] has %d variables.", getOwnerObject().getNetworkId().getValueString().c_str(), variableCount));
	data.reserve(static_cast<ByteVector::size_type>(3 + variableCount * 2));
	data.push_back(static_cast<byte>(variableCount));

	//-- Write per-variable data.
	for (int i = 0; i < variableCount; ++i)
	{
		CustomizationVariable const *variable = variables[static_cast<CustomizationVariableConstVector::size_type>(i)].second;
		NOT_NULL(variable);

		std::string const &variableName = variables[static_cast<CustomizationVariableConstVector::size_type>(i)].first;
		DEBUG_FATAL(variableName.length() < 1, ("zero-length variable name: logic error."));

		// Lookup variable id from name.
		int variableId = -1;
		bool const mapSuccessful = CustomizationIdManager::mapStringToId(variableName.c_str(), variableId);
		
		FATAL(!mapSuccessful, ("object id [%s] failed to map customization variable name [%s] to a customization variable id. Somebody forgot to run the customization id update process. Will not try to save incomplete data.", getOwnerObject().getNetworkId().getValueString().c_str(), variableName.c_str()));
		DEBUG_FATAL((variableId < 0) || (variableId >= 128), ("logic error: CustomizationIdManager returned an out-of-range value [%d] for customization data format 0.", variableId));

		// Find out if variable is stored as 1 byte unsigned or 2 byte signed.
		int const persistedDataSize = variable->getPersistedDataByteCount();
		VALIDATE_RANGE_INCLUSIVE_INCLUSIVE(1, persistedDataSize, 2);

		//-- Write variable declaration.
		byte combinedId = static_cast<byte>(variableId);
		if (persistedDataSize == 2)
			combinedId |= 0x80;
		
		data.push_back(combinedId);

		//-- Write variable data.
		variable->saveToByteVector(data);
	}
}

// ----------------------------------------------------------------------

bool CustomizationData::restoreFromByteVector(ByteVector const &data)
{
	Object const &object = getOwnerObject();

	//-- Check for no data.
	if (data.empty())
	{
		WARNING(true, ("object id [%s], template [%s] tried to restore customization data from zero-length customization data.", object.getNetworkId().getValueString().c_str(), object.getObjectTemplateName()));
		return false;
	}

	//-- Get version number.
	bool returnValue = false;
	
	byte const version = data.front();
	switch (version)
	{
		case 1:
			returnValue = restoreFromByteVector_1(data);
			break;

		case 2:
			returnValue = restoreFromByteVector_2(data);
			break;

		default:
		{
			WARNING(true, ("object id [%s], template [%s] tried to restore customization data from unsupported version [%d].", object.getNetworkId().getValueString().c_str(), object.getObjectTemplateName(), static_cast<int>(version)));
			returnValue = false;
		}
	}

	//-- Signal that we are modified.
	if (returnValue)
		signalModified();

	return returnValue;
}

// ----------------------------------------------------------------------

bool CustomizationData::restoreFromByteVector_1(ByteVector const &data)
{
	Object const &object = getOwnerObject();

		//-- Verify data contains a variable count.
	int const dataSize = static_cast<int>(data.size());
	if (dataSize < 2)
	{
		WARNING(true, ("object id [%s], template [%s] tried to restore customization data from corrupt customization data version 1.", object.getNetworkId().getValueString().c_str(), object.getObjectTemplateName()));
		return false;
	}

	//-- Get variable count.
	int const variableCount = static_cast<int>(data[1]);
	int const minPossibleDataSize = 2 + variableCount * 2;
	if (dataSize < minPossibleDataSize)
	{
		WARNING(true, ("object id [%s], template [%s] customization data corrupted: must be at least [%d] bytes for [%d] variables but only contains [%d] bytes.", object.getNetworkId().getValueString().c_str(), object.getObjectTemplateName(), minPossibleDataSize, variableCount, dataSize));
		return false;
	}

	bool result = true;

	//-- Process each variable.
	int currentIndex = 2;
	std::string  variableName;
	
	for (int i = 0; i < variableCount; ++i)
	{
		// Get combined variable id.
		byte const combinedVariableId = data[static_cast<ByteVector::size_type>(currentIndex)];
		++currentIndex;

		// Extract high-bit to determine if variable is interpreted as 2-byte (signed) or 1-byte (unsigned),
		// extract lower 7 bits for variable id.
		int const variableSize = ((combinedVariableId & 0x80) != 0) ? 2 : 1;
		int const variableId   = combinedVariableId & 0x7f;

		// Lookup variable name.
		bool const foundMapping = CustomizationIdManager::mapIdToString(variableId, variableName);
		if (!foundMapping)
		{
			WARNING(true, ("object id [%s], template [%s] references unmapped variable, id=[%d], variable size = [%d], aborting load.", object.getNetworkId().getValueString().c_str(), object.getObjectTemplateName(), variableId, variableSize));
			result = false;
			return false;
		}

		// Get variable for name.
		CustomizationVariable *const variable = findVariable(variableName);
		if (!variable)
		{
			WARNING(true, ("object id [%s], template [%s] references undeclared variable [%s], ignoring variable.", object.getNetworkId().getValueString().c_str(), object.getObjectTemplateName(), variableName.c_str()));
			currentIndex += variableSize;
			result = false;
			continue;
		}

		// Let variable load the data.
		bool const restoreSuccess = variable->restoreFromByteVector(data, currentIndex, variableSize);
		if (!restoreSuccess)
		{
			WARNING(true, ("object id [%s], template [%s] references customization variable [%s]; data restoration failed.", object.getNetworkId().getValueString().c_str(), object.getObjectTemplateName(), variableName.c_str()));
			result = false;
			return false;
		}

		// Move past data.
		currentIndex += variableSize;
	}

	return result;
}

// ----------------------------------------------------------------------

bool CustomizationData::restoreFromByteVector_2(ByteVector const &data)
{
	// The binary representation hasn't changed between versions 1 & 2.
	// Only the string representation has changed because of an utf8 issue
	// (we use utf8 to store the string in an objvar but the version 1
	//  string had "characters" that were incompatible with utf8)
	return restoreFromByteVector_1(data);
}

// ======================================================================
