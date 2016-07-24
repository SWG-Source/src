// ======================================================================
//
//
// Copyright 6/19/2001 Sony Online Entertainment
//
// ======================================================================


#include "FirstArchive.h"
#include "ArchiveMutex.h"

namespace Archive
{

ArchiveMutex::ArchiveMutex()
{
	InitializeCriticalSection(&m_criticalSection);
}

ArchiveMutex::~ArchiveMutex()
{
	DeleteCriticalSection(&m_criticalSection);
}

void ArchiveMutex::enter()
{
	EnterCriticalSection(&m_criticalSection);
}

void ArchiveMutex::leave()
{
	LeaveCriticalSection(&m_criticalSection);
}

}
