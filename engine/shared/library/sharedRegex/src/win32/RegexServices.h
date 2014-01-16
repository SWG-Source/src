// ======================================================================
//
// RegexServices.h
// Copyright 2003 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#ifndef INCLUDED_RegexServices_H
#define INCLUDED_RegexServices_H

// ======================================================================

class RegexServices
{
public:

	static void *allocateMemory(size_t byteCount);
	static void  freeMemory(void *pointer);

};

// ======================================================================

#endif
