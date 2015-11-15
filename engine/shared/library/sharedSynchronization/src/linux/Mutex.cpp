// ======================================================================
//
// Mutex.cpp
// Acy Stapp
//
// Copyright 6/19/2001 Sony Online Entertainment
//
// ======================================================================

#include "sharedFoundation/FirstSharedFoundation.h"
#include "sharedSynchronization/Mutex.h"

Mutex::Mutex()
{
	pthread_mutex_init(&mutex, 0);
}

Mutex::~Mutex()
{
	pthread_mutex_destroy(&mutex);
}

void Mutex::enter()
{
	pthread_mutex_lock(&mutex);
}

void Mutex::leave()
{
	pthread_mutex_unlock(&mutex);
}

