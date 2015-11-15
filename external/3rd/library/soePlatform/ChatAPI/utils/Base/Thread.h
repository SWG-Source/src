#ifndef BASE_THREAD_H
#define BASE_THREAD_H

#ifdef WIN32

    #include "win32/Thread.h"

#elif linux

    #include "linux/Thread.h"

#elif sparc

    #include "solaris/Thread.h"

#else

    #error /Base/Thread.h: Undefine platform type

#endif

#endif  //  BASE_THREAD_H
