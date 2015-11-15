#ifndef BASE_EVENT_H
#define BASE_EVENT_H

#ifdef WIN32

    #include "win32/Event.h"

#elif linux

    #include "linux/Event.h"

#elif sparc

    #include "solaris/Event.h"

#else

    #error /Base/Event.h: Undefine platform type

#endif

#endif  //  BASE_EVENT_H
