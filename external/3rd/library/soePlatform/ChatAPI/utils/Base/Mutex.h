#ifndef	BASE_MUTEX_H
#define	BASE_MUTEX_H

#ifdef WIN32

    #include "win32/Mutex.h"

#elif linux

    #include "linux/Mutex.h"

#elif sparc

    #include "solaris/Mutex.h"

#else

    #error /Base/Mutex.h: Undefine platform type

#endif

#endif  //  BASE_MUTEX_H

