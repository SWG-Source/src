#ifndef BASE_TYPES_H
#define BASE_TYPES_H

#ifdef WIN32
#	define FILENAME (strrchr(__FILE__, '\\') ? strrchr(__FILE__, '\\') + 1 : __FILE__)
#else
#	define FILENAME __FILE__
#endif

#ifdef WIN32

    #include "win32/Types.h"

#elif linux

    #include "linux/Types.h"

#elif sparc

    #include "solaris/Types.h"

#else

    #error /Base/Types.h: Undefine platform type

#endif

#endif 

