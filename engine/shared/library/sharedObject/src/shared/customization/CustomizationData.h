// ======================================================================
//
// CustomizationData.h
// Copyright 2002, 2003 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#ifndef INCLUDED_CustomizationData_H
#define INCLUDED_CustomizationData_H

// ======================================================================

#include "sharedFoundation/MemoryBlockManagerMacros.h"

class CrcLowerString;
class CustomizationVariable;
class Object;

// ======================================================================
/**
 * Provides access to customization data.
 *
 * Typically a CustomizationData instance is embedded in a
 * CustomizationDataProperty.  That property is attached to
 * an Object instance.
 *
 * Appearance-derived classes make use of the CustomizationData to
 * customize the look of an appearance based on the values of customization
 * variables.
 *
 * CustomizationData variables resides in a directory structure much like a
 * Unix filesystem.  A CustomizationData instance can mount the directory
 * of another CustomizationData instance.  The mounting feature is used by
 * wearables to grab the body size customization variables from the current
 * owner of the wearable.
 *
 * @see CustomizationDataProperty
 * @see Object
 */

class CustomizationData
{
	MEMORY_BLOCK_MANAGER_INTERFACE_WITHOUT_INSTALL;

public:

	class Directory;
	class LocalDirectory;   // should be private by mbm macros needs it to be public.
	
	class RemoteDirectory;  // should be private by mbm macros needs it to be public.
	friend class RemoteDirectory;
	

	typedef void (*ModificationCallback)(const CustomizationData &customizationData, const void *context);
	typedef void (*ConstIteratorCallback)(const std::string &fullVariablePathName, const CustomizationVariable *customizationVariable, void *context);
	typedef void (*IteratorCallback)(const std::string &fullVariablePathName, CustomizationVariable *customizationVariable, void *context);

	typedef std::vector<byte>  ByteVector;
	
public:

	static void         install();

	static int          parseSeparatedHexInt(const std::string &data, int startPosition, int &endPosition);
	static std::string  parseSeparatedString(const std::string &data, int startPosition, int &endPosition);

	static std::string  makeSelectDumpFormat(char const *source);
	
public:

	explicit CustomizationData(Object &owner);

	void                         addVariableTakeOwnership(const std::string &fullVariablePathName, CustomizationVariable *variable);

	const CustomizationVariable *findConstVariable(const std::string &fullVariablePathName) const;
	CustomizationVariable       *findVariable(const std::string &fullVariablePathName);

	void                         iterateOverConstVariables(ConstIteratorCallback callback, void *context, bool includeRemoteVariables = true) const;
	void                         iterateOverVariables(IteratorCallback callback, void *context, bool includeRemoteVariables = true);

	bool                         mountRemoteCustomizationData(CustomizationData &customizationDataToAttach, const std::string &remoteDirectory, const std::string &localDirectory, bool nonExistentLeafDirectoryOk = false);
	bool                         dismountRemoteCustomizationData(const std::string &localDirectoryPathName, bool nonExistentLeafDirectoryOk = false);

	const CustomizationData     *getDirectoryOwner(const std::string &directory) const;

	std::string                  writeLocalDataToString() const;
	void                         loadLocalDataFromString(const std::string &stringData);
	
	void                         registerModificationListener(ModificationCallback modificationCallback, const void *context);
	void                         deregisterModificationListener(ModificationCallback modificationCallback, const void *context);

	void                         signalModified() const;

	void                         fetch() const;
	void                         release() const;
	int                          getReferenceCount() const;

	Object                      &getOwnerObject();
	const Object                &getOwnerObject() const;

	void                         alterLocalVariables();

#ifdef _DEBUG
	void                         debugDump() const;
#endif

public:

	static const char  cms_directorySeparator;
	static const char  cms_stringFieldSeparator;

	static bool        ms_installed;

private:

	struct ModificationCallbackData;

	typedef std::map<CustomizationData*, int>                       CustomizationDataIntMap;
	typedef std::vector<ModificationCallbackData>                   ModificationCallbackDataVector;

private:

	static void remove();

#ifdef _DEBUG
	static void debugDumpIterationCallback(const std::string &fullVariablePathName, const CustomizationVariable *customizationVariable, void *context);
#endif

private:

	~CustomizationData();

	const Directory *findConstDirectoryFromPathName(const std::string &variablePathName, bool traverseRemoteDirectories, int &directoryEntryStartIndex) const;
	Directory       *findDirectoryFromPathName(const std::string &variablePathName, bool traverseRemoteDirectories, bool createLocalMissingDirectories, int &directoryEntryStartIndex);

	void             addDependentCustomizationData(CustomizationData *customizationData);
	void             removeDependentCustomizationData(CustomizationData *customizationData);

	void             notifyPendingRemoteDestruction(const CustomizationData *customizationData);

	void             loadLocalDataFromString_1(const std::string &stringData);
	void             loadLocalDataFromString_2(const std::string &stringData);

	void             saveToByteVector(ByteVector &data, bool persistRemoteData = false) const;

	bool             restoreFromByteVector(ByteVector const &data);
	bool             restoreFromByteVector_1(ByteVector const &data);
	bool             restoreFromByteVector_2(ByteVector const &data);

	
	// Disabled.
	CustomizationData();
	CustomizationData(const CustomizationData&);
	CustomizationData &operator =(const CustomizationData&);

private:

	mutable int                     m_referenceCount;

	Object                         &m_owner;
	Directory                      *m_rootDirectory;

	CustomizationDataIntMap        *m_dependentCustomizationDataMap;
	ModificationCallbackDataVector *m_modificationCallbacks;

};

// ======================================================================
/**
 * Increment the reference count associated with the CustomizationData
 * instance.
 *
 * The user should fetch() a reference for each logical reference the
 * user makes to the CustomizationData instance.  When a logical reference
 * no longer is needed, the caller should call release().
 *
 * @see CustomizationData::release()
 */

inline void CustomizationData::fetch() const
{
	++m_referenceCount;
}

// ----------------------------------------------------------------------
/**
 * Retrieve the reference count.
 *
 * @return  reference count.
 */

inline int CustomizationData::getReferenceCount() const
{
	return m_referenceCount;
}

// ----------------------------------------------------------------------

inline Object &CustomizationData::getOwnerObject()
{
	return m_owner;
}

// ----------------------------------------------------------------------

inline const Object &CustomizationData::getOwnerObject() const
{
	return m_owner;
}

// ======================================================================

#endif
