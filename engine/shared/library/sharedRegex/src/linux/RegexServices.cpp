// ======================================================================
//
// RegexServices.cpp
// Copyright 2003 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#include "sharedRegex/FirstSharedRegex.h"
#include "sharedRegex/RegexServices.h"

// ======================================================================

void *RegexServices::allocateMemory(size_t byteCount)
{
	return new char[byteCount];
}

// ----------------------------------------------------------------------

void RegexServices::freeMemory(void *pointer)
{
	delete [] reinterpret_cast<char *>(pointer);
}

// ======================================================================
