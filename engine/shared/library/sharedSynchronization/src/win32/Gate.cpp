// ======================================================================
//
// Gate.cpp
//
// Copyright 2001-2002 Sony Online Entertainment
// All Rights Reserved.
//
// ======================================================================

#include "sharedSynchronization/FirstSharedSynchronization.h"
#include "sharedSynchronization/Gate.h"

// ======================================================================

Gate::Gate(bool open)
{
	handle = CreateEvent(0, true, open, 0);
	DEBUG_FATAL(handle == NULL, ("CreateEvent failed"));
}

// ----------------------------------------------------------------------

Gate::~Gate()
{
	CloseHandle(handle);
}

// ----------------------------------------------------------------------

void Gate::wait()
{
	int errors = 0;
	for (;;)
	{
		DWORD result = WaitForSingleObject(handle, INFINITE);
		if (result ==  WAIT_OBJECT_0)
			return;

		++errors;
		FATAL(errors >= 3, ("WaitForSingleObject failed multiple times"));
	}
}

// ----------------------------------------------------------------------

void Gate::close()
{
	const BOOL result = ResetEvent(handle);
	FATAL(result == 0, ("ResetEvent failed"));
}

// ----------------------------------------------------------------------

void Gate::open()
{
	const BOOL result = SetEvent(handle);
	FATAL(result == 0, ("SetEvent failed"));
}

// ======================================================================
