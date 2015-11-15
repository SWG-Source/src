// ======================================================================
//
// WindowsWrapper.h
// copyright (c) 1998 Bootprint Entertainment
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_WindowsWrapper_H
#define INCLUDED_WindowsWrapper_H

// ======================================================================

// C4201 nonstandard extension used : nameless struct/union
#pragma warning(disable: 4201)

// make windows.h more strict in the types of handles
#ifndef STRICT
#define STRICT 1
#endif

// trim down the amount of stuff windows.h includes
#define NOGDICAPMASKS
#define NOVIRTUALKEYCODE
#define NOKEYSTATES
#define NORASTEROPS
#define NOATOM
#define NOCOLOR
#define NODRAWTEXT
#define NOMEMMGR
#define NOMETAFILE
#define NOMINMAX
#define NOOPENFILE
#define NOSERVICE
#define NOSOUND
#define NOCOMM
#define NOHELP
#define NOPROFILER
#define NODEFERWINDOWPOS
#define NOMCX
#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <wtypes.h>

// reenable warnings disables for windows.h
#pragma warning(default: 4201)

// ======================================================================

#endif
