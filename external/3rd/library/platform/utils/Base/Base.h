////////////////////////////////////////
//  Base.h
//
//  Purpose:
// 	    1. Provide a single header file that may be used to include all
//          Base library header files.
//
//  Revisions:
// 	    07/10/2001  Created                                    
//

#ifndef _BASEPCH_H
#define _BASEPCH_H

#ifdef WIN32

    #include "win32/Platform.h"
    #include "win32/Types.h"
    #include "win32/Mutex.h"
    #include "win32/Event.h"
    #include "win32/Thread.h"
    #include "Logger.h"
    #include "Config.h"
    #include "ScopeLock.h"
    #include "Statistics.h"
    #include "TemplateObjectAllocator.h"
    #include "BlockAllocator.h"

#elif linux

    #include "linux/Platform.h"
    #include "linux/Types.h"
    #include "linux/Mutex.h"
    #include "linux/Event.h"
    #include "linux/Thread.h"
    #include "Logger.h"
    #include "Config.h"
    #include "ScopeLock.h"
    #include "Statistics.h"
    #include "TemplateObjectAllocator.h"
    #include "BlockAllocator.h"

#elif sparc

    #include "solaris/Platform.h"
    #include "solaris/Types.h"
    #include "solaris/Mutex.h"
    #include "solaris/Event.h"
    #include "solaris/Thread.h"
    #include "Logger.h"
    #include "Config.h"
    #include "ScopeLock.h"
    #include "Statistics.h"
    #include "TemplateObjectAllocator.h"
    #include "BlockAllocator.h"

#else

    #error Base.h: Undefine platform type

#endif

#endif  //  _BASEPCH_H
