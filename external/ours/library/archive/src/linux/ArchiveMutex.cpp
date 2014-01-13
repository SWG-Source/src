// ======================================================================
//
//
// Copyright 6/19/2001 Sony Online Entertainment
//
// ======================================================================


#include "ArchiveMutex.h"

namespace Archive
{

ArchiveMutex::ArchiveMutex()
{
	pthread_mutex_init(&mutex, 0);
}

ArchiveMutex::~ArchiveMutex()
{
	pthread_mutex_destroy(&mutex);
}

void ArchiveMutex::enter()
{
	pthread_mutex_lock(&mutex);
}

void ArchiveMutex::leave()
{
	pthread_mutex_unlock(&mutex);
}

}
