// ======================================================================
//
// RegistryKey.cpp
// Todd Fiala
//
// copyright 1998 Bootprint Entertainment
//
// ======================================================================

#include "sharedFoundation/FirstSharedFoundation.h"
#include "sharedFoundation/RegistryKey.h"

#include "sharedFoundation/ExitChain.h"
#include "sharedFoundation/ConfigFile.h"

#if USE_REGISTRY_EXECUTION_STATISTICS
#include <cstdio>
#include <ctime>
#endif

// ======================================================================

// keyname for config file parameter containing relative registry path
// for the product

const char * const RegistryKey::PRODUCT_REGISTRY_PATH_KEYNAME = "ProductRegistryPath";
const char * const RegistryKey::DEFAULT_PRODUCT_REGISTRY_PATH = "Software\\Sony Online Entertainment\\Default";

#if USE_REGISTRY_EXECUTION_STATISTICS
const char * const RegistryKey::STATISTICS_KEYNAME                   = "Statistics";
const char * const RegistryKey::STAT_EXEC_STARTED_COUNT_VALUENAME    = "StartedExecutions";
const char * const RegistryKey::STAT_EXEC_INCOMPLETE_COUNT_VALUENAME = "IncompleteExecutions";
const char * const RegistryKey::STAT_TIME_AVERAGE_VALUENAME          = "AverageClockTime";
const char * const RegistryKey::STAT_TIME_MIN_VALUENAME              = "MinimumClockTime";
const char * const RegistryKey::STAT_TIME_MAX_VALUENAME              = "MaximumClockTime";
const char * const RegistryKey::STAT_TIME_START_VALUENAME            = "StartClockTime";

#endif

bool         RegistryKey::installed;
bool         RegistryKey::setProductKeyPathFromConfig;

RegistryKey *RegistryKey::usersKey;
RegistryKey *RegistryKey::currentUserKey;
RegistryKey *RegistryKey::classRootKey;
RegistryKey *RegistryKey::localMachineKey;
RegistryKey *RegistryKey::productUserKey;
RegistryKey *RegistryKey::productMachineKey;

// ======================================================================
// construct a RegistryKey instance
//
// Remarks:
//   This function does not create the underlying registry object.  It
//   solely attaches such a registry object with a RegistryKey class
//   instance.

RegistryKey::RegistryKey(
	HKEY newKeyHandle,        // [IN] handle of existing registry key
	bool newCloseKeyOnDestroy // [IN] true if RegCloseKey() should be called on key at destruction time
	) :
	keyHandle(newKeyHandle),
	closeKeyOnDestroy(newCloseKeyOnDestroy)
{
	// -qq- don't know which of these indicates an invalid handle, so check for both
	DEBUG_FATAL(!newKeyHandle, ("null newKeyHandle arg"));
	DEBUG_FATAL(newKeyHandle == INVALID_HANDLE_VALUE, ("newKeyHandle arg is an invalid handle"));
}

// ----------------------------------------------------------------------
/**
 * destroy a RegistryKey instance.
 */

RegistryKey::~RegistryKey(void)
{
	DEBUG_FATAL(!keyHandle, ("null keyHandle"));

	if (closeKeyOnDestroy)
	{
		const LONG result = RegCloseKey(keyHandle);
		UNREF(result);
		DEBUG_FATAL(result != ERROR_SUCCESS, ("failed to close registry key, error = %ld\n", result));
	}
	keyHandle = 0;
}

// ----------------------------------------------------------------------
/**
 * enumerate all child subkeys of the current key.
 *
 * @param context  [IN] user-specified context variable passed to callback
 * @param callback  [IN] function to call for each subkey enumerated
 * @see EnumerateKeyCallback, enumerateValues()
 */

void RegistryKey::enumerateSubkeys(void *context, EnumerateKeyCallback callback) const
{
	const    size_t MAX_NAME_LENGTH = 256;

	LONG     result;
	FILETIME lastModifiedFileTime;
	char     subkeyName[MAX_NAME_LENGTH];
	DWORD    subkeyNameLength;
	DWORD    index;

	DEBUG_FATAL(!callback, ("null callback arg\n"));

	for (
		index = 0, subkeyNameLength = MAX_NAME_LENGTH;
	  ERROR_SUCCESS == (result = RegEnumKeyEx(keyHandle, index, subkeyName, &subkeyNameLength, 0, 0, 0, &lastModifiedFileTime));
		++index, subkeyNameLength = MAX_NAME_LENGTH
		)
	{
		bool continueEnum = callback(context, subkeyName, &lastModifiedFileTime);
		if (!continueEnum) break;
	}

	// ensure we finished enumeration cleanly
	FATAL(
		(result != ERROR_SUCCESS) && (result != ERROR_NO_MORE_ITEMS),
		("failed to enumerate registry subkeys, error = %ld\n", result));
}

// ----------------------------------------------------------------------
/**
 * open and return a subkey under this RegistryKey instance.
 *
 * The function will fail if the specified subkey does not exist.
 *
 * @param subkeyName  [IN] registry path for subkey to open, relative to this RegistryKey instance
 * @param accessFlags  [IN] flags indicating access granted to the returned RegistryKey
 * @return This function returns a RegistryKey instance for the specified
 * subkey.
 */

RegistryKey *RegistryKey::openSubkey(const char *subkeyName, uint32 accessFlags) const
{
	REGSAM samDesired = 0;
	LONG   result;
	HKEY   newKey = 0;

	DEBUG_FATAL(!keyHandle, ("null keyHandle"));
	DEBUG_FATAL(!subkeyName, ("null subkeyName arg"));
	DEBUG_FATAL(!strlen(subkeyName), ("zero-length subkeyName arg"));

	// configure security access desired
	if (accessFlags & AF_READ)
		samDesired |= KEY_READ;
	if (accessFlags & AF_WRITE)
		samDesired |= KEY_WRITE;

	// open the key
	result = RegOpenKeyEx(keyHandle, subkeyName, 0, samDesired, &newKey);
	FATAL(result != ERROR_SUCCESS, ("failed to open registry subkey \"%s\", error = %ld\n", subkeyName, result));

	// Create the RegistryKey object.  Assume the key must be closed upon
	// destruction.
	return new RegistryKey(newKey, true);
}

// ----------------------------------------------------------------------
/**
 * create and return a subkey under this RegistryKey instance.
 *
 * The function will succeed even if the specified subkey
 * already exists.
 *
 * @return This function returns a RegistryKey instance for the specified
 * subkey.
 */

RegistryKey *RegistryKey::createSubkey(const char *subkeyName,	uint32 accessFlags) const
{
	REGSAM samDesired = 0;
	LONG   result;
	HKEY   newKey = 0;
	DWORD  disposition;

	DEBUG_FATAL(!keyHandle, ("null keyHandle"));
	DEBUG_FATAL(!subkeyName, ("null subkeyName arg"));
	DEBUG_FATAL(!strlen(subkeyName), ("zero-length subkeyName arg"));

	// configure security access desired
	if (accessFlags & AF_READ)
		samDesired |= KEY_READ;
	if (accessFlags & AF_WRITE)
		samDesired |= KEY_WRITE;

	// open the key
	result = RegCreateKeyEx(
		keyHandle,
		subkeyName,
		0,                       // reserved
		const_cast<char *>(""),  // class
		REG_OPTION_NON_VOLATILE, // options
		samDesired,
		NULL,                    // security
		&newKey,
		&disposition);
	FATAL(result != ERROR_SUCCESS, ("failed to create registry subkey \"%s\", error = %ld\n", subkeyName, result));

	// Create the RegistryKey object.  Assume the key must be closed upon
	// destruction.
	return new RegistryKey(newKey, true);
}

// ----------------------------------------------------------------------
/**
 * delete a subkey relative to this RegistryKey instance.
 *
 * Under Windows NT, deleting a key that has subkeys is considered an
 * error.  Therefore, the client should ensure the target key for
 * deletion does not have any subkeys.  (Under Windows 9X, deletion of
 * the target registry key will automatically delete all subkeys
 * underneath the target key.)
 */

void RegistryKey::deleteSubkey(const char *subkeyName)
{
	DEBUG_FATAL(!keyHandle, ("null keyHandle"));
	DEBUG_FATAL(!subkeyName, ("null subkeyName arg"));
	DEBUG_FATAL(!strlen(subkeyName), ("zero-length subkeyName arg"));

	IGNORE_RETURN(RegDeleteKey(keyHandle, subkeyName));
}

// ----------------------------------------------------------------------
/**
 * test for the existence of a subkey under this RegistryKey instance.
 *
 * @param subkeyName  [IN] registry path for subkey to open, relative to this RegistryKey instance
 */

bool RegistryKey::subKeyExists(const char *subkeyName)
{
	REGSAM samDesired = KEY_READ;
	LONG   result;
	HKEY   newKey = 0;

	DEBUG_FATAL(!keyHandle, ("null keyHandle"));
	DEBUG_FATAL(!subkeyName, ("null subkeyName arg"));
	DEBUG_FATAL(!strlen(subkeyName), ("zero-length subkeyName arg"));

	// open the key
	result = RegOpenKeyEx(keyHandle, subkeyName, 0, samDesired, &newKey);
	return (result == ERROR_SUCCESS);
}

// ----------------------------------------------------------------------
/**
 * enumerate all values under the current key.
 *
 * @param context  [IN] user-specified context variable passed to callback
 * @param callback  [IN] function to call for each value enumerated
 * @see EnumerateValueCallback, enumerateSubkeys()
 */

void RegistryKey::enumerateValues(void *context, EnumerateValueCallback callback) const
{
	const    size_t MAX_NAME_LENGTH = 256;

	LONG     result;
	char     valueName[MAX_NAME_LENGTH];
	DWORD    valueNameLength;
	DWORD    index;
	DWORD    dataSize;
	DWORD    valueType;

	DEBUG_FATAL(!callback, ("null callback arg\n"));

	for (
		index = 0, valueNameLength = MAX_NAME_LENGTH, dataSize = 0;
	  ERROR_SUCCESS == (result = RegEnumValue(keyHandle, index, valueName, &valueNameLength, 0, &valueType, 0, &dataSize));
		++index, valueNameLength = MAX_NAME_LENGTH, dataSize = 0
		)
	{
		bool continueEnum = callback(context, valueName, static_cast<uint32>(dataSize), valueType);
		if (!continueEnum) break;
	}

	// ensure we finished enumeration cleanly
	FATAL(
		(result != ERROR_SUCCESS) && (result != ERROR_NO_MORE_ITEMS),
		("failed to enumerate registry values, error = %ld\n", result));
}

// ----------------------------------------------------------------------
/**
 * query if a value exists and how many bytes that value occupies.
 *
 * The client does can pass null for the valueSize and/or valueType
 * parameter if that information is not desired.  The value returned
 * within the valueSize and valueType parameter is only valid
 * if the doesExist parameter is set to true upon function return.
 * The value of the valueType parameter is one of the Windows-defined
 * REG_* constants provided for use with the Win32 Reg* registry
 * API.  Common values are REG_BINARY (binary data), REG_DWORD
 * (unsigned integral type data) and REG_SZ (C-style zero-terminated
 * strings).  Consult the Win32 documentation for RegGetValueEx()
 * for a description of all the type constants.
 *
 * @param valueName  [IN] name of the value to query
 * @param doesExist  [OUT] true if value exists under this key, false otherwise
 * @param valueSize  [OUT] number of bytes occupied by value's data
 * @param valueType  [OUT] type flag for registry value (see REG_* in RegSetValueEx())
 */

void RegistryKey::getValueInfo(const char *valueName, bool *doesExist, uint32 *valueSize, DWORD *valueType) const
{
	LONG  result;
	DWORD dwSize = 0;

	DEBUG_FATAL(!keyHandle, ("null keyHandle"));
	DEBUG_FATAL(!valueName, ("null valueName arg"));
	DEBUG_FATAL(!doesExist, ("null doesExist arg"));
	DEBUG_FATAL(!strlen(valueName), ("zero-length valueName arg"));

	// get information on the value
	result = RegQueryValueEx(keyHandle, valueName, 0, valueType, 0, &dwSize);

	// assume a query failure indicates the value was not found
	*doesExist = (result == ERROR_SUCCESS);
	if (valueSize)
		*valueSize = static_cast<uint32>(dwSize);
}

// ----------------------------------------------------------------------
/**
 * set the data for this key's value.
 *
 * The value of the valueType parameter is one of the Windows-defined
 * REG_* constants provided for use with the Win32 Reg* registry
 * API.  Common values are REG_BINARY (binary data), REG_DWORD
 * (unsigned integral type data) and REG_SZ (C-style zero-terminated
 * strings).  Consult the Win32 documentation for RegGetValueEx()
 * for a description of all the type constants.
 *
 * @param valueName  [IN] name of value under which data will be stored
 * @param dataPtr  [IN] pointer to buffer containing value's data
 * @param dataSize  [IN] # bytes to store
 * @param valueType  [IN] type field for value (indicates nature of data stored with value)
 */

void RegistryKey::setValue(const char *valueName, const void *dataPtr, uint32 dataSize, DWORD valueType)
{
	LONG  result;

	DEBUG_FATAL(!keyHandle, ("null keyHandle"));
	DEBUG_FATAL(!valueName, ("null valueName arg"));
	DEBUG_FATAL(!strlen(valueName), ("zero-length valueName arg"));
	DEBUG_FATAL(!dataPtr, ("null dataPtr arg"));

	// get information on the value
	result = RegSetValueEx(keyHandle, valueName, 0, valueType, static_cast<CONST BYTE*>(dataPtr), static_cast<DWORD>(dataSize));
	FATAL(result != ERROR_SUCCESS, ("failed to set registry value \"%s\", error = %ld\n", valueName, result));
}

// ----------------------------------------------------------------------
/**
 * retrieve the data for the this key's value.
 *
 * The valueType and/or the valueDataSize parameter may be set
 * to null if the client doesn't need this information.
 * The value of the valueType parameter is one of the Windows-defined
 * REG_* constants provided for use with the Win32 Reg* registry
 * API.  Common values are REG_BINARY (binary data), REG_DWORD
 * (unsigned integral type data) and REG_SZ (C-style zero-terminated
 * strings).  Consult the Win32 documentation for RegGetValueEx()
 * for a description of all the type constants.
 *
 * @param valueName  [IN] the name of the value to retrieve
 * @param dataPtr  [OUT] the buffer where the value's data will be stored
 * @param maxDataSize  [IN] size of the data buffer in bytes
 * @param valueDataSize  [OUT] number of bytes retrieved from the named value
 * @param valueType  [OUT] type field for value's data (see REG_* codes in RegSetValueEx)
 */

void RegistryKey::getValue(const char *valueName, void *dataPtr, uint32 maxDataSize, uint32 *valueDataSize, DWORD *valueType) const
{
	LONG  result;
	DWORD dwSize = maxDataSize;

	DEBUG_FATAL(!keyHandle, ("null keyHandle"));
	DEBUG_FATAL(!valueName, ("null valueName arg"));
	DEBUG_FATAL(!strlen(valueName), ("zero-length valueName arg"));

	// get information on the value
	result = RegQueryValueEx(keyHandle, valueName, 0, valueType, static_cast<LPBYTE>(dataPtr), &dwSize);
	FATAL(result != ERROR_SUCCESS, ("failed to read registry value \"%s\", error = %ld\n", valueName, result));

	// assume a query failure indicates the value was not found
	if (valueDataSize)
		*valueDataSize = static_cast<uint32>(dwSize);
}

// ----------------------------------------------------------------------
/**
 * retrieve a string from of this key's values, returning a default string
 * if the value doesn't exist.
 *
 * The defaultValue arg is required, must be non-null.
 *
 * If the specified value exists but is not of type REG_SZ, the default
 * string will be returned.
 *
 * This routine will DEBUG_FATAL on null args, even if optional is set.
 * Optional only controls whether a FATAL occurs if there is an issue
 * retrieving the value.
 *
 * @param valueName  [IN]  name of the string value to retrieve
 * @param defaultValue  [IN]  string to return if registry key does not contain thev value
 * @param dest  [OUT] buffer for returned string
 * @param destSize  [IN]  max number of bytes (including terminating null) that can be placed in dest
 * @param optional  [IN]  if set, the function will return false if there is a problem retrieving the value.  otherwise, a DEBUG_FATAL will occur
 */

bool RegistryKey::getStringValue(const char *valueName, const char *defaultValue, char *dest, DWORD destSize, bool optional)
{
	DEBUG_FATAL(!defaultValue, ("null defaultValue arg"));
	DEBUG_FATAL(!dest, ("null dest arg"));

	bool   doesExist;
	DWORD  valueType;
	DWORD  valueSize;

	// clear out string
	memset(dest, 0, static_cast<size_t>(destSize));

	// get info on the value
	getValueInfo (valueName, &doesExist, &valueSize, &valueType);
	if (!doesExist || (valueType != REG_SZ))
		strncpy(dest, defaultValue, destSize-1);
	else
	{
		// value exists and is a string, retrieve it
		getValue (valueName, dest, destSize, &valueSize);
		if (!valueSize)
		{
			FATAL(!optional, ("failed to get registry data for key %s\n", valueName));
			return false;
		}

		// terminate the string
		dest [valueSize] = 0;
	}
	return true;
}


// ----------------------------------------------------------------------
/**
 * delete a value from this registry key.
 */

void RegistryKey::deleteValue(const char *valueName)
{
	LONG result;

	DEBUG_FATAL(!keyHandle, ("null keyHandle"));
	DEBUG_FATAL(!valueName, ("null valueName arg"));
	DEBUG_FATAL(!strlen(valueName), ("zero-length valueName arg"));

	result = RegDeleteValue(keyHandle, valueName);
	FATAL(result != ERROR_SUCCESS, ("failed to delete registry value \"%s\", error = %ld\n", valueName, result));
}

#if USE_REGISTRY_EXECUTION_STATISTICS

// ----------------------------------------------------------------------
/**
 * updates beginning-of-execution statistics using the values under a
 * given key.
 *
 * Call this function at the beginning of execution of the code
 * for which statistics are desired.  At the end of execution,
 * call updateShutdownKey() to finish generating statistics.
 * This function should only be called once per execution,
 * and should be followed by a call to updateShutdownKey().
 * If the latter is not called, the next call to updateStartupKey()
 * will assume the application terminated abnormally before
 * proper shutdown could occur.
 */

void RegistryKey::updateStartupKey(RegistryKey *key)
{
	bool    valueExist;
	uint32  valueSize;
	DWORD   valueType;
	DWORD   execStartedCount = 0;
	clock_t startTime = clock();

	DEBUG_FATAL(!key, ("null key arg\n"));

	// handle improper shutdown of last run
	key->getValueInfo(STAT_TIME_START_VALUENAME, &valueExist);
	if (valueExist)
	{
		// appears the product did not run to completion on last run.
		// increment the incomplete run counts

		DWORD execIncompleteCount = 0;

		key->getValueInfo(STAT_EXEC_INCOMPLETE_COUNT_VALUENAME, &valueExist);
		if (valueExist)
		{
			key->getValue(STAT_EXEC_INCOMPLETE_COUNT_VALUENAME, &execIncompleteCount, sizeof(DWORD), &valueSize, &valueType);
			DEBUG_FATAL((valueType != REG_DWORD) || (valueSize != sizeof(DWORD)), ("%s key not DWORD type as expected\n", STAT_EXEC_INCOMPLETE_COUNT_VALUENAME));
		}
		++execIncompleteCount;
		key->setValue(STAT_EXEC_INCOMPLETE_COUNT_VALUENAME, &execIncompleteCount, sizeof(DWORD), REG_DWORD);
	}

	// get the start count
	key->getValueInfo(STAT_EXEC_STARTED_COUNT_VALUENAME, &valueExist);
	if (valueExist)
	{
		key->getValue(STAT_EXEC_STARTED_COUNT_VALUENAME, &execStartedCount, sizeof(DWORD), &valueSize, &valueType);
		DEBUG_FATAL((valueType != REG_DWORD) || (valueSize != sizeof(DWORD)), ("%s key not DWORD type as expected\n", STAT_EXEC_STARTED_COUNT_VALUENAME));
	}
	++execStartedCount;

	// save the data
	key->setValue(STAT_EXEC_STARTED_COUNT_VALUENAME, &execStartedCount, sizeof(DWORD), REG_DWORD);
	key->setValue(STAT_TIME_START_VALUENAME, &startTime, sizeof(clock_t));
}

#endif


#if USE_REGISTRY_EXECUTION_STATISTICS

// ----------------------------------------------------------------------
/**
 * updates end-of-execution statistics using the values under a given key.
 *
 * This function should only be called after updateStartupKey()
 * has been called on the same key.  After this call returns,
 * this function should not be called again until another
 * call to updateStartupKey() is made.
 */

void RegistryKey::updateShutdownKey(RegistryKey *key)
{
	bool    valueExist;
	uint32  valueSize;
	DWORD   valueType;
	clock_t minTime;
	clock_t maxTime;
	clock_t averageTime = 0;
	clock_t startTime;
	clock_t stopTime = clock();
	clock_t runTime;
	DWORD   startExecCount;
	DWORD   incompleteExecCount = 0;
	DWORD   completeExecCount;
	real    averageFraction;

	DEBUG_FATAL(!key, ("null key arg\n"));

	// find the number of complete execution runs of the product
	key->getValue(STAT_EXEC_STARTED_COUNT_VALUENAME, &startExecCount, sizeof(DWORD), &valueSize, &valueType);
	DEBUG_FATAL((valueSize != sizeof(DWORD)) || (valueType != REG_DWORD), ("invalid start count statistic registry value\n"));

	key->getValueInfo(STAT_EXEC_INCOMPLETE_COUNT_VALUENAME, &valueExist);
	if (valueExist)
	{
		key->getValue(STAT_EXEC_INCOMPLETE_COUNT_VALUENAME, &incompleteExecCount, sizeof(DWORD), &valueSize, &valueType);
		DEBUG_FATAL((valueType != REG_DWORD) || (valueSize != sizeof(DWORD)), ("%s expected to be DWORD type, was %lu instead\n", STAT_EXEC_INCOMPLETE_COUNT_VALUENAME, valueType));
	}
	completeExecCount = startExecCount - incompleteExecCount;

	// retrieve the start time
	key->getValue(STAT_TIME_START_VALUENAME, &startTime, sizeof(clock_t), &valueSize, &valueType);
	DEBUG_FATAL((valueSize != sizeof(clock_t)) || (valueType != REG_BINARY), ("invalid start time statistic registry value\n"));

	// -qq- we don't handle clock wraparound
	runTime     = stopTime - startTime;
	minTime     = runTime;
	maxTime     = runTime;

	// calculate average time
	key->getValueInfo(STAT_TIME_AVERAGE_VALUENAME, &valueExist);
	if (valueExist)
	{
		key->getValue(STAT_TIME_AVERAGE_VALUENAME, &averageTime, sizeof(clock_t), &valueSize, &valueType);
		DEBUG_FATAL((valueSize != sizeof(clock_t)) || (valueType != REG_BINARY), ("invalid average exec time statistic registry value\n"));
	}
	averageFraction = CONST_REAL(1.0 / completeExecCount);
	averageTime = static_cast<clock_t>(( CONST_REAL(averageTime) * (completeExecCount-1) + runTime) * averageFraction);
	if (averageTime < 1)
		averageTime = 1;

	// check min exec time
	key->getValueInfo(STAT_TIME_MIN_VALUENAME, &valueExist);
	if (valueExist)
	{
		clock_t testValue;
		key->getValue(STAT_TIME_MIN_VALUENAME, &testValue, sizeof(clock_t), &valueSize, &valueType);
		DEBUG_FATAL((valueSize != sizeof(clock_t)) || (valueType != REG_BINARY), ("invalid min exec time statistic registry value\n"));
		if (testValue < minTime)
			minTime = testValue;
	}

	// check for max exec time
	key->getValueInfo(STAT_TIME_MAX_VALUENAME, &valueExist);
	if (valueExist)
	{
		clock_t testValue;
		key->getValue(STAT_TIME_MAX_VALUENAME, &testValue, sizeof(clock_t), &valueSize, &valueType);
		DEBUG_FATAL((valueSize != sizeof(clock_t)) || (valueType != REG_BINARY), ("invalid max exec time statistic registry value\n"));
		if (testValue > maxTime)
			maxTime = testValue;
	}

	// save the data
	key->setValue(STAT_TIME_AVERAGE_VALUENAME, &averageTime, sizeof(clock_t));
	key->setValue(STAT_TIME_MIN_VALUENAME, &minTime, sizeof(clock_t));
	key->setValue(STAT_TIME_MAX_VALUENAME, &maxTime, sizeof(clock_t));

	// delete the start time value
	key->deleteValue(STAT_TIME_START_VALUENAME);
}

#endif


#if USE_REGISTRY_EXECUTION_STATISTICS

// ----------------------------------------------------------------------
/**
 * An EnumValueCallback that prints each value name, type and size
 *
 * @param valueName name of value
 * @param valueSize size of value in bytes
 * @param valueTye type of value
 */

bool RegistryKey::enumValueInfoPrint(
	void*,                 //  unused user context
	const char *valueName,
	uint32 valueSize,
	DWORD valueType
	)
{
	char *valueTypeStr;
	char buffer[64];

	switch (valueType)
	{
	case REG_BINARY:
		valueTypeStr = "REG_BINARY";
		break;
	case REG_DWORD:
		valueTypeStr = "REG_DWORD [little endian]";
		break;
	case REG_DWORD_BIG_ENDIAN:
		valueTypeStr = "REG_DWORD_BIG_ENDIAN";
		break;
	case REG_EXPAND_SZ:
		valueTypeStr = "REG_EXPAND_SZ";
		break;
	case REG_NONE:
		valueTypeStr = "REG_NONE";
		break;
	case REG_SZ:
		valueTypeStr = "REG_SZ";
		break;
	default:
		valueTypeStr = buffer;
		sprintf(buffer, "<TYPE %lu>", valueType);
		break;
	}

	DEBUG_PRINT_LOG(true, ("  Key: \"%s\", %s, %u bytes\n", valueName, valueTypeStr, valueSize));

	// continue enumerating
	return true;
}

#endif


#if USE_REGISTRY_EXECUTION_STATISTICS

// ----------------------------------------------------------------------
/**
 * Update per-startup registry values.
 *
 * This routine is used primarily to test registry read/write
 * functionality; however, it does provide statistics related
 * to game usage.  These include the following statistics on
 * a per-game and per-machine basis:
 * # times product was executed
 * average real-time duration of execution
 * minimum execution time
 * maximum execution time
 * # times product failed to execute to completion
 * (i.e. updateRegistryShutdown() never executed---hard crash, stop debugging)
 */

void RegistryKey::updateStartupStatistics(void)
{
	DEBUG_FATAL(!productUserKey, ("null productUserKey\n"));
	DEBUG_FATAL(!productMachineKey, ("null productMachineKey\n"));

	// create required keys
	RegistryKey *userStatKey    = productUserKey->createSubkey(STATISTICS_KEYNAME);
	RegistryKey *machineStatKey = productMachineKey->createSubkey(STATISTICS_KEYNAME);

	FATAL(!userStatKey, ("failed to create user statistics key"));
	FATAL(!machineStatKey, ("failed to create machine statistics key"));

	// handle machine stat key

#if 1
	// print existing machine stat keys
	DEBUG_PRINT_LOG(true, ("BEGIN machine stat key enumeration:\n"));
	machineStatKey->enumerateValues(0, enumValueInfoPrint);
	DEBUG_PRINT_LOG(true, ("END machine stat key enumeration:\n"));
#endif

	updateStartupKey(machineStatKey);


	// handle user stat key

#if 1
	// print existing user stat keys
	DEBUG_PRINT_LOG(true, ("BEGIN user stat key enumeration:\n"));
	userStatKey->enumerateValues(0, enumValueInfoPrint);
	DEBUG_PRINT_LOG(true, ("END user stat key enumeration:\n"));
#endif

	updateStartupKey(userStatKey);

	// cleanup
	delete machineStatKey;
	delete userStatKey;
}

#endif


#if USE_REGISTRY_EXECUTION_STATISTICS

// ----------------------------------------------------------------------
/**
 * Update per-shutdown registry values.
 *
 * @see updateRegistryStartup()
 */

void RegistryKey::updateShutdownStatistics(void)
{
	DEBUG_FATAL(!productUserKey, ("null productUserKey\n"));
	DEBUG_FATAL(!productMachineKey, ("null productMachineKey\n"));

	// create required keys
	RegistryKey *userStatKey    = productUserKey->createSubkey(STATISTICS_KEYNAME);
	RegistryKey *machineStatKey = productMachineKey->createSubkey(STATISTICS_KEYNAME);

	FATAL(!userStatKey, ("failed to create user statistics key"));
	FATAL(!machineStatKey, ("failed to create machine statistics key"));

	// handle machine stat key
	updateShutdownKey(machineStatKey);

	// handle user stat key
	updateShutdownKey(userStatKey);

	// cleanup
	delete machineStatKey;
	delete userStatKey;
}

#endif

// ----------------------------------------------------------------------
/**
 * install the RegistryKey class.
 *
 * After installation, the following predefined keys are available:
 *
 * usersKey:           HKEY_USERS
 * currentUserKey:     HKEY_CURRENT_USER
 * classRootKey:       HKEY_CLASSES_ROOT
 * localMachineKey:    HKEY_LOCAL_MACHINE
 * productUserKey:     HKEY_CURENT_USER\<product relative registry path>
 * productMachineKey:  HKEY_LOCAL_MACHINE\<product relative registry path>
 *
 * The client is free to create any other keys required.
 *
 * The product relative registry path is determined by the first of the
 * following methods that succeed:
 * 1.  If the config file key "ProductRegistryPath" is set, that value is
 * used for the relative path.  If the product relative path is set
 * in this manner, the client's attempt to change the product relative
 * path via the setProductKey() function will silently fail.
 * 2.  If the productKeyRelativePath parameter is a non-null positive length
 * string, that value is used as the product keys' relative path.
 * 3.  The default value DEFAULT_PRODUCT_REGISTRY_PATH ("Software\\Bootprint\\Default")
 * is used.
 *
 * RegistryKey::remove() is added to the exit chain.
 *
 * Do not delete any of the predefined registry keys.
 *
 * @param productKeyRelativePath  [IN] relative registry path for the product keys
 * @see setProductKeyPath()
 */

void RegistryKey::install(const char *productKeyRelativePath)
{
	UNREF(productKeyRelativePath);
	char *useRegistryPath = 0;

	DEBUG_FATAL(installed, ("attempted to install RegistryKey when already installed\n"));
	installed = true;

	// add to exit chain
	ExitChain::add(remove, "RegistryKey::remove", 0, true);

	// if no passed in value, use the default
	setProductKeyPathFromConfig = false;
 	useRegistryPath = DuplicateString(DEFAULT_PRODUCT_REGISTRY_PATH);

	// create the Windows-defined RegistryKeys (these keys don't get closed at object destruction time)
	usersKey        = new RegistryKey(HKEY_USERS, false);         //lint !e1924 // Note -- C-style cast
	currentUserKey  = new RegistryKey(HKEY_CURRENT_USER, false);  //lint !e1924 // Note -- C-style cast
	classRootKey    = new RegistryKey(HKEY_CLASSES_ROOT, false);  //lint !e1924 // Note -- C-style cast
	localMachineKey = new RegistryKey(HKEY_LOCAL_MACHINE, false); //lint !e1924 // Note -- C-style cast

	// check for errors --- unnecessary if new cannot return NULL
	FATAL(
		!usersKey || !currentUserKey || !classRootKey || !localMachineKey,
		("failed to create standard RegistryKey objects"));

	// create/open the product user key
	productUserKey = currentUserKey->createSubkey(useRegistryPath, AF_READ | AF_WRITE);

	// create/open the product machine key
	productMachineKey = localMachineKey->createSubkey(useRegistryPath, AF_READ);

	delete [] useRegistryPath;

#if USE_REGISTRY_EXECUTION_STATISTICS
	updateStartupStatistics();
#endif
}

// ----------------------------------------------------------------------
/**
 * release all resources and state associated with the RegistryKey system.
 *
 * It is invalid to call any RegistryKey functions other than install()
 * after calling remove().
 */

void RegistryKey::remove(void)
{
#if USE_REGISTRY_EXECUTION_STATISTICS
	updateShutdownStatistics();
#endif

	DEBUG_FATAL(!installed, ("attempted to remove RegistryKey when not installed\n"));
	installed = false;

	delete productMachineKey;
	delete productUserKey;
	delete localMachineKey;
	delete classRootKey;
	delete currentUserKey;
	delete usersKey;

	productMachineKey = 0;
	productUserKey    = 0;
	localMachineKey   = 0;
	classRootKey      = 0;
	currentUserKey    = 0;
	usersKey          = 0;

}

// ----------------------------------------------------------------------
/**
 * reset the path to the product's machine and user keys.
 *
 * This routine will redefine the following keys:
 * productUserKey:     HKEY_CURENT_USER\<productKeyRelativePath>
 * productMachineKey:  HKEY_LOCAL_MACHINE\<productKeyRelativePath>
 *
 * If the product key relative paths were specified in the config
 * file, this routine will essentially do nothing --- the client
 * is not able to ovveride the config file settings.
 */

void RegistryKey::setProductKeyPath(const char *productKeyRelativePath)
{
	DEBUG_FATAL(
		!productKeyRelativePath || !productKeyRelativePath[0],
		("invalid productKeyRelativePath arg\n"));

	// don't allow client to overwrite if path set by config file
	if (setProductKeyPathFromConfig)
	{
		DEBUG_REPORT_LOG_PRINT(true, ("attempted to set product registry key path to '%s' but config file setting will override\n", productKeyRelativePath));
		return;
	}

	delete productUserKey;
	delete productMachineKey;

	productUserKey    = currentUserKey->createSubkey(productKeyRelativePath, AF_READ | AF_WRITE);
	productMachineKey = localMachineKey->createSubkey(productKeyRelativePath, AF_READ | AF_WRITE);
}

// ======================================================================
