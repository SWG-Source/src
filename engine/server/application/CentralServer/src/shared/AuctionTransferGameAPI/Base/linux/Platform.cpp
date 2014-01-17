////////////////////////////////////////
//  Platform.cpp
//
//  Purpose:
// 	    1. Implementation of the global functionality declaired in Platform.h.
//
//  Revisions:
// 	    07/10/2001  Created                                    
//

#include <ctype.h>
#include "Platform.h"

#ifdef EXTERNAL_DISTRO
namespace NAMESPACE 
{

#endif
namespace Base
{

    // Implementation of microsoft strlwr extension
    // This non-ANSI function is not supported under UNIX
    void strlwr(char * s)
    {
        while (*s)
        {
            *s = tolower(*s);
          s++;
        }
    }

    // Implementation of microsoft strlwr extension
    // This non-ANSI function is not supported under UNIX
    void strupr(char * s)
    {
        while (*s)
        {
            *s = toupper(*s);
          s++;
        }
    }


    CTimer::CTimer() :
        mTimer(0)
    {
    }


}
#ifdef EXTERNAL_DISTRO
};
#endif

