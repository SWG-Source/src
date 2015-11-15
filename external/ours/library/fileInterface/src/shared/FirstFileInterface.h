// ======================================================================
//
// FirstFileInterface.h
// Copyright 2002, Sony Online Entertainment Inc.
// All Rights Reserved.
//
// ======================================================================

#ifndef INCLUDED_FirstFileInterface_H
#define INCLUDED_FirstFileInterface_H

// ======================================================================

typedef unsigned char byte;

#ifdef _WIN32

#pragma warning(disable: 4291 4514 4702 4710 4786)

void * __cdecl operator new(size_t size);
void * __cdecl operator new[](size_t size);

void operator delete(void *pointer);
void operator delete[](void *pointer);

#endif

// ======================================================================

#endif
