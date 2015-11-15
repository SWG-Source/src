////////////////////////////////////////
//  Platform.cpp
//
//  Purpose:
// 	    1. Implementation of the global functionality declaired in Platform.h.
//
//  Revisions:
// 	    07/10/2001  Created                                    
//

#include "Platform.h"

#ifdef EXTERNAL_DISTRO
namespace NAMESPACE 
{

#endif
namespace Base 
{


    CTimer::CTimer() :
        mTimer(0)
    {
    }


};
#ifdef EXTERNAL_DISTRO
};
#endif
