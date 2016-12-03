// ======================================================================
//
// FirstPlatform.h
// jeff grills
//
// copyright 1998 Bootprint Entertainment
//
// ======================================================================

#ifndef FIRST_PLATFORM_H
#define FIRST_PLATFORM_H

// ======================================================================

#include <cstdlib>

#define DLLEXPORT

// ======================================================================

// C4514 unreferenced inline function has been removed
// C4710 inline function not expanded
// C4291 no matching operator delete found; memory will not be freed if initialization throws an exception

#include <float.h>
#include <math.h>
#include <ctype.h>
#include <pthread.h>
#include <semaphore.h>
#include <stdarg.h>
#include <string.h>
#include <wchar.h>

#include "sharedFoundation/PlatformGlue.h"



// ======================================================================

template <class T>
inline int ComGetReferenceCount(T *t)
{
	t->AddRef();
	return t->Release();
}

// ======================================================================

#define FATAL_HR(a,b)       FATAL(FAILED(b), (a, HRESULT_CODE(b)))
#define DEBUG_FATAL_HR(a,b) DEBUG_FATAL(FAILED(b), (a, HRESULT_CODE(b)))

// ======================================================================

#endif
