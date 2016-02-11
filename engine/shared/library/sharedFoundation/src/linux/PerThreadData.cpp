// ======================================================================
//
// PerThreadData.cpp
// jeff grills
//
// copyright 1998 Bootprint Entertainment
//
// ======================================================================

#include "sharedFoundation/FirstSharedFoundation.h"
#include "sharedFoundation/PerThreadData.h"

#include "sharedSynchronization/Gate.h"

// ======================================================================

pthread_key_t PerThreadData::slot;
bool PerThreadData::slotCreated=false;

// ======================================================================
// Install the per-thread-data subsystem
//
// Remarks:
//
//   This routine will install the per-thread-data subsystem that is required for several
//   other subsystems in the engine.  It should be called from the primary thread before
//   any other threads have been created.  It will also call threadInstall() for the
//   primary thread.
//
// See Also:
//
//   PerThreadData::remove()

void PerThreadData::install(void)
{
	if (pthread_key_create(&slot, 0)!=0)
	{
		FATAL(true, ("pthread_key_create failed"));
	}
	slotCreated=true;
	
	threadInstall();
}

// ----------------------------------------------------------------------
/**
 * Remove the per-thread-subsystem.
 * 
 * This routine should be called by the primary thread after all other threads have
 * terminated, and no other uses of per-thread-data will occur.
 * 
 * @see PerThreadData::install()
 */

void PerThreadData::remove(void)
{
	threadRemove();

	if (pthread_key_delete(slot)!=0)
	{
		FATAL(true, ("pthread_key_delete failed"));
	}

	slotCreated=false;
}

// ----------------------------------------------------------------------
/**
 * Get access to the per-thread-data.
 * 
 * This routine will verify the per-thread-data subsystem has been installed and the
 * threadInstall() function has been called for the current thread.
 * 
 * @return A pointer to the per-thread-data.
 */

PerThreadData::Data *PerThreadData::getData(bool allowReturnNull)
{
	UNREF(allowReturnNull);

	if (!slotCreated)
	{
		DEBUG_FATAL(true && !allowReturnNull, ("not installed"));
		return nullptr;
	}

	Data * const data = reinterpret_cast<Data *>(pthread_getspecific(slot));
	DEBUG_FATAL(!data && !allowReturnNull, ("not installed for this thread"));
	return data;
}

// ----------------------------------------------------------------------
/**
 * Create the per-thread-data for a new thread.
 * 
 * This routine should be called in a thread before the first usage of per-thread-data.
 * 
 * If the client is calling this function on a thread that existed before the engine was
 * installed, the client should set isNewThread to false.  Setting isNewThread to false
 * prevents the function from validating that the thread's TLS is nullptr.  For threads existing
 * before the engine is installed, the TLS data for the thread is undefined.
 * 
 * @param isNewThread  [IN] true if the thread was created after the engine was installed, false otherwise
 * @see PerThreadData::threadRemove()
 */

void PerThreadData::threadInstall(bool isNewThread)
{
	UNREF(isNewThread);

	DEBUG_FATAL(!slotCreated, ("not installed"));

	// only check for already-set data if this is supposed to be a new thread
	DEBUG_FATAL(isNewThread && pthread_getspecific(slot), ("already installed for this thread"));

	// create the data
	Data * const data = new Data;

	// initialize the data
	memset(data, 0, sizeof(*data));

	//create the event for file streaming reads
	data->readGate  = new Gate(false);

	// set the data into the thread slot
	const BOOL result = pthread_setspecific(slot, data);
	UNREF(result);
	DEBUG_FATAL(result, ("pthread_setspecific failed")); //NB:  unlike Windows, returns 0 on success.
} //lint !e429 // Warning -- Custodial pointer 'data' has not been freed or returned)  // stored int thread-local-storage

// ----------------------------------------------------------------------
/**
 * Destroythe per-thread-data for a terminating thread.
 * 
 * This routine should be called in a thread after the last usage of per-thread-data.
 * 
 * @see PerThreadData::threadInstall()
 */

void PerThreadData::threadRemove(void)
{
	DEBUG_FATAL(!slotCreated, ("not installed"));
	DEBUG_FATAL(!pthread_getspecific(slot), ("thread not installed"));

	// get the data
	Data *data = getData();

	//close the event used for file streaming reads
	delete data->readGate;
	data->readGate = nullptr;

	// wipe the data in the thread slot
	const BOOL result2 = pthread_setspecific(slot, nullptr);
	UNREF(result2);
	DEBUG_FATAL(result2, ("TlsSetValue failed")); //NB:  unlike Windows, returns 0 on success.

	// free the memory
	delete data;
}

// ======================================================================
