// ======================================================================
//
// RecursiveMutex.cpp
//
// Copyright 2001-2002 Sony Online Entertainment
// All Rights Reserved
//
// ======================================================================

#include "sharedSynchronization/FirstSharedSynchronization.h"
#include "sharedSynchronization/RecursiveMutex.h"

// ======================================================================

void RecursiveMutex::install()
{
}

// ----------------------------------------------------------------------

void RecursiveMutex::remove()
{
}

// ======================================================================

RecursiveMutex::RecursiveMutex()
{
	InitializeCriticalSection(&m_criticalSection);
}

// ----------------------------------------------------------------------

RecursiveMutex::~RecursiveMutex()
{
	DeleteCriticalSection(&m_criticalSection);
}

// ----------------------------------------------------------------------

void RecursiveMutex::enter()
{
	EnterCriticalSection(&m_criticalSection);
}

// ----------------------------------------------------------------------

void RecursiveMutex::leave()
{
	LeaveCriticalSection(&m_criticalSection);
}

// ======================================================================
