// FirstSharedNetwork.h
// Copyright 2000-02, Sony Online Entertainment Inc., all rights reserved. 

//-----------------------------------------------------------------------

#ifndef	INCLUDED_FirstSharedNetwork_H
#define	INCLUDED_FirstSharedNetwork_H

//-----------------------------------------------------------------------

//#pragma warning ( disable : 4514 ) // unreferenced inline function has been removed (STL)
//#pragma warning ( disable : 4702 ) // unreachable code (STL)

// FirstSharedFoundation includes windows.h; keep its legacy Winsock 1.1
// declarations out of this Winsock 2 library.
#if defined(WIN32) && !defined(WIN32_LEAN_AND_MEAN)
#define WIN32_LEAN_AND_MEAN
#endif

#include "sharedFoundation/FirstSharedFoundation.h"
#if defined(WIN32)
#include <winsock2.h>
#endif
#include "sharedDebug/FirstSharedDebug.h"


#include <string>

//-----------------------------------------------------------------------

#endif	// INCLUDED_FirstSharedNetwork_H
