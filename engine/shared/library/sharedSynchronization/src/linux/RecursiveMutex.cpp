// ======================================================================
//
// RecursiveMutex.cpp
// Acy Stapp
//
// Copyright 6/19/2001 Sony Online Entertainment
//
// ======================================================================

#include "sharedFoundation/FirstSharedFoundation.h"
#include "sharedSynchronization/RecursiveMutex.h"

RecursiveMutex::RecursiveMutex()
{
 	pthread_mutexattr_t attributes;
 
	pthread_mutexattr_init(&attributes);
	pthread_mutexattr_settype(&attributes,PTHREAD_MUTEX_RECURSIVE);
	pthread_mutex_init(&mutex,&attributes);
	pthread_mutexattr_destroy(&attributes);
}

RecursiveMutex::~RecursiveMutex()
{
	pthread_mutex_destroy(&mutex);
}

void RecursiveMutex::enter()
{
	pthread_mutex_lock(&mutex);
}

void RecursiveMutex::leave()
{
	pthread_mutex_unlock(&mutex);
}

