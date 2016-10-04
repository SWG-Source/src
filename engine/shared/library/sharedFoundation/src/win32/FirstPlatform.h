// ======================================================================
//
// FirstPlatform.h
// copyright 1998 Bootprint Entertainment
// copyright 2001 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_FirstPlatform_H
#define INCLUDED_FirstPlatform_H


#ifdef WIN32

//fucks up use of std::min std::max
#define NOMINMAX 

#include <windows.h>

#ifndef __wtypes_h__
#include <wtypes.h>
#endif

#ifndef __WINDEF_
#include <windef.h>
#endif

#include <WinNT.h>

//laziness
#include <stdio.h>
#include <cstdlib>

#endif

// ======================================================================

// C4127 conditional expression is constant
// C4291 no matching operator delete found; memory will not be freed if initialization throws an exception
// C4503 decorated name length exceeded, name was truncated
// C4514 unreferenced inline function has been removed
// C4702 unreachable code
// C4710 inline function not expanded
// C4786 identifier was truncated to 'number' characters in the debug

#pragma warning(disable: 4127 4291 4503 4514 4702 4710 4786)

// ======================================================================
// If we haven't defined this yet, then we're not compiling a DLL

#ifndef  COMPILE_DLL
#define  COMPILE_DLL 0
#endif

#if COMPILE_DLL
#define DLLEXPORT __declspec(dllimport)
#else
#define DLLEXPORT __declspec(dllexport)
#endif

// ======================================================================

template <class T>
inline int ComGetReferenceCount(T *t)
{
	t->AddRef();
	return t->Release();
}

// ======================================================================
// forward declare some windows stuff to avoid having to include <windows.h> here

struct HCURSOR__;
struct HICON__;
struct HINSTANCE__;
struct HWND__;

typedef void           *HANDLE;
typedef HICON__        *HCURSOR;
typedef HICON__        *HICON;
typedef HINSTANCE__    *HINSTANCE;
typedef HWND__         *HWND;

// @todo codereorg still working on this
#include "sharedFoundation/WindowsWrapper.h"

// ======================================================================
// convienent fatal macros that check windows HRESULT codes

#define FATAL_HR(a,b)       FATAL(FAILED(b), (a, HRESULT_CODE(b)))
#define DEBUG_FATAL_HR(a,b) DEBUG_FATAL(FAILED(b), (a, HRESULT_CODE(b)))

// ======================================================================
// include anything we need to replace missing functionality that other platforms provide

#include "sharedFoundation/PlatformGlue.h"

// ======================================================================

#endif
