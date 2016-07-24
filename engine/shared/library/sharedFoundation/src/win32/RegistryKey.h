#ifndef REGISTRY_KEY_H
#define REGISTRY_KEY_H

// ======================================================================
//
// RegistryKey.h
// Todd Fiala
//
// copyright 1998 Bootprint Entertainment
//
// ======================================================================

// provide read and write access to the Windows Registry
//
// Remarks:
//
//   Each RegistryKey can create or open a subkey relative to that
//   key.  This process produces another RegistryKey.  RegistryKeys
//   provide a mechanism to enumerate all the child subkeys and values
//   associated with the key.  In addition, clients may query for
//   the presence, type and size of a specific value, get a specific
//   value's data, and set a named value's data and type.
//
//   The RegistryKey class provides the client with six RegistryKey
//   objects as starting points into the registry.  See documentation
//   for install() for more info on these starting points.
//
//   The client must install() the RegistryKey class before using it.

class RegistryKey
{
public:
	// ----------------------------------------------------------------------
	// flags used to indicate what type of access (e.g. read, write) should
	// be granted when creating or opening a key.
	//
	// See Also:
	//
	//   createSubkey(), openSubkey()

	enum // AccessFlags
	{
		AF_READ  = BINARY1(0001), // query values, enumerate subkeys
		AF_WRITE = BINARY1(0010)  // set values, create subkeys
	};

	// ----------------------------------------------------------------------
	// callback prototype for subkey enumeration
	//
	// Return Value:
	//
	//   A return value of true indicates the enumeration of subkeys should
	//   continue.  A return value of false stops further enumeration of 
	//   subkeys.
	//
	// Remarks:
	//
	//   The context parameter is specified in the call to 
	//   enumerateSubkeys().  One use for this could be to pass a class
	//   instance, with the callack defined as a static member function.  
	//   The function can then cast the context to a pointer to class
	//   instance, allowing the static function to access per-instance
	//   data explicitly in place of an assumed this pointer.
	//   
	// See Also:
	//
	//   enumerateSubkeys()

	typedef bool (*EnumerateKeyCallback)(
		void *context,                 // [IN] user-specified context variable
		const char *keyName,           // [IN] name of the key being enumerated
		const FILETIME *lastWriteTime  // [IN] last time this key was modified
		);

	// ----------------------------------------------------------------------
	// callback prototype for value enumeration
	//
	// Return Value:
	//
	//   A return value of true indicates the enumeration of values should
	//   continue.  A return value of false stops further enumeration of 
	//   values .
	//
	// Remarks:
	//
	//   The context parameter is specified in the call to 
	//   enumerateValues().  One use for this could be to pass a class
	//   instance, with the callack defined as a static member function.  
	//   The function can then cast the context to a pointer to class
	//   instance, allowing the static function to access per-instance
	//   data explicitly in place of an assumed this pointer.
	//   
	// See Also:
	//
	//   enumerateValues()

	typedef bool (*EnumerateValueCallback)(
		void *context,                 // [IN] user-specified context variable
		const char *valueName,         // [IN] name of the value being enumerated
		uint32 valueSize,              // [IN] number of bytes occupied by this value's data
		DWORD valueType                // [IN] type of value (one of REG_* as in RegSetValueEx documentation)
		);

private:
	static const char * const PRODUCT_REGISTRY_PATH_KEYNAME;
	static const char * const DEFAULT_PRODUCT_REGISTRY_PATH;

#if USE_REGISTRY_EXECUTION_STATISTICS
	static const char * const STATISTICS_KEYNAME;
	static const char * const STAT_EXEC_STARTED_COUNT_VALUENAME;
	static const char * const STAT_EXEC_INCOMPLETE_COUNT_VALUENAME;
	static const char * const STAT_TIME_AVERAGE_VALUENAME;
	static const char * const STAT_TIME_MIN_VALUENAME;
	static const char * const STAT_TIME_MAX_VALUENAME;
	static const char * const STAT_TIME_START_VALUENAME;
#endif
	
	static bool installed;
	static bool setProductKeyPathFromConfig;

	// private instance member variables
	HKEY  keyHandle;
	bool  closeKeyOnDestroy;

private:
	// public member variables
	static RegistryKey *usersKey;
	static RegistryKey *currentUserKey;
	static RegistryKey *classRootKey;
	static RegistryKey *localMachineKey;
	static RegistryKey *productUserKey;
	static RegistryKey *productMachineKey;

private:
	// private member functions
	static void remove(void);
	RegistryKey(HKEY newKeyHandle, bool newCloseKeyOnDestroy);

#if USE_REGISTRY_EXECUTION_STATISTICS
	static void updateStartupKey(RegistryKey *key);
	static void updateShutdownKey(RegistryKey *key);
	static bool enumValueInfoPrint(void *context, const char *valueName, uint32 valueSize, DWORD valueType);

	static void updateStartupStatistics(void);
	static void updateShutdownStatistics(void);
#endif

private:
	// disable: default constructor, copy constructor, assignment operator
	// NOTE: last two can be implemented, but must be specially handled
	RegistryKey(void);
	RegistryKey(const RegistryKey&);
	RegistryKey &operator =(const RegistryKey&);

public:
	// public member functions

	static void install(const char *productKeyRelativePath = 0);
	static void setProductKeyPath(const char *productKeyRelativePath);

	// pre-defined RegistryKey retrieval
	static RegistryKey *getUsersKey(void);
	static RegistryKey *getCurrentUserKey(void);
	static RegistryKey *getClassRootKey(void);
	static RegistryKey *getLocalMachineKey(void);
	static RegistryKey *getProductUserKey(void);
	static RegistryKey *getProductMachineKey(void);

	~RegistryKey(void);

	// subkey interface
	void         enumerateSubkeys(void *context, EnumerateKeyCallback callback) const;
	RegistryKey *openSubkey(const char *subkeyName, uint32 accessFlags = AF_READ) const;
	RegistryKey *createSubkey(const char *subkeyName, uint32 accessFlags = AF_READ | AF_WRITE) const;
	void         deleteSubkey(const char *subkeyName);
	bool         subKeyExists(const char *subkeyName);

	// value interface
	void  enumerateValues(void *context, EnumerateValueCallback callback) const;
	void  setValue(const char *valueName, const void *dataPtr, uint32 dataSize, DWORD valueType = REG_BINARY);
	void  deleteValue(const char *valueName);

	void  getValueInfo(const char *valueName, bool *doesExist, uint32 *valueSize = 0, DWORD *valueType = 0) const;
	void  getValue(const char *valueName, void *dataPtr, uint32 maxDataSize, uint32 *valueDataSize, DWORD *valueType = 0) const;
	bool  getStringValue(const char *valueName, const char *defaultValue, char *dest, DWORD destSize, bool optional = false);

};

// ======================================================================
// retrieve a key to the HKEY_USERS registry key with read access

inline RegistryKey *RegistryKey::getUsersKey(void)
{
	DEBUG_FATAL(!installed, ("Attempted to use RegistryKey when not installed\n"));
	return usersKey;
}

// ----------------------------------------------------------------------
/**
 * retrieve a key to the HKEY_CURRENT_USER registry key with read/write
 * access.
 */

inline RegistryKey *RegistryKey::getCurrentUserKey(void)
{
	DEBUG_FATAL(!installed, ("Attempted to use RegistryKey when not installed\n"));
	return currentUserKey;
}

// ----------------------------------------------------------------------
/**
 * retrieve a key to the HKEY_CLASSES_ROOT registry key with read/write
 * access.
 */

inline RegistryKey *RegistryKey::getClassRootKey(void)
{
	DEBUG_FATAL(!installed, ("Attempted to use RegistryKey when not installed\n"));
	return classRootKey;
}

// ----------------------------------------------------------------------
/**
 * retrieve a key to the HKEY_LOCAL_MACHINE registry key with read/write
 * access.
 */

inline RegistryKey *RegistryKey::getLocalMachineKey(void)
{
	DEBUG_FATAL(!installed, ("Attempted to use RegistryKey when not installed\n"));
	return localMachineKey;
}

// ----------------------------------------------------------------------
/**
 * retrieve a key to the product's user key with read/write access.
 * 
 * The product user key is the root key for storing user-specific
 * product-related information.  install() describes where this
 * information exists in the registry.
 * 
 * @see install(), getProductMachineKey()
 */

inline RegistryKey *RegistryKey::getProductUserKey(void)
{
	DEBUG_FATAL(!installed, ("Attempted to use RegistryKey when not installed\n"));
	return productUserKey;
}

// ----------------------------------------------------------------------
/**
 * retrieve a key to the product's machine key with read/write access.
 * 
 * The product machine key is the root key for storing machine-specific
 * product-related information.  install() describes where this
 * information exists in the registry.
 * 
 * @see install(), getProductUserKey()
 */

inline RegistryKey *RegistryKey::getProductMachineKey(void)
{
	DEBUG_FATAL(!installed, ("Attempted to use RegistryKey when not installed\n"));
	return productMachineKey;
}

// ======================================================================

#endif
