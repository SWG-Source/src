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

namespace Base
{

    // Implementation of microsoft strlwr extension
    // This non-ANSI function is not supported under UNIX
    void _strlwr(char * s)
    {
        while (*s)
        {
            *s = tolower(*s);
          s++;
        }
    }

    // Implementation of microsoft strupr extension
    // This non-ANSI function is not supported under UNIX
    void _strupr(char * s)
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