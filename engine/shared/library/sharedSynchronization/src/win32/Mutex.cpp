// ======================================================================
//
// Mutex.cpp
//
// Copyright 2001-2002 Sony Online Entertainment
// All Rights Reserved
//
// ======================================================================

#include "sharedSynchronization/FirstSharedSynchronization.h"
#include "sharedSynchronization/Mutex.h"

// ======================================================================

void Mutex::install()
{
}

// ----------------------------------------------------------------------

void Mutex::remove()
{
}

// ======================================================================

Mutex::Mutex()
{
	InitializeCriticalSection(&m_criticalSection);
}

// ----------------------------------------------------------------------

Mutex::~Mutex()
{
	DeleteCriticalSection(&m_criticalSection);
}

// ----------------------------------------------------------------------

void Mutex::enter()
{
	EnterCriticalSection(&m_criticalSection);
}

// ----------------------------------------------------------------------

void Mutex::leave()
{
	LeaveCriticalSection(&m_criticalSection);
}

// ======================================================================
