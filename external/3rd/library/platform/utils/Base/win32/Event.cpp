////////////////////////////////////////
//  Event.cpp
//
//  Purpose:
// 	    1. Implementation of the CEvent class.
//
//  Revisions:
// 	    07/10/2001  Created                                    
//


#if !defined(_MT)
#   pragma message( "Excluding Base::CEvent - requires multi-threaded compile. (_MT)" ) 
#else

#include "Event.h"

#ifdef EXTERNAL_DISTRO
namespace NAMESPACE 
{

#endif
namespace Base 
{

    CEvent::CEvent()
    {
        mEvent = CreateEvent( NULL, FALSE, FALSE, NULL );
    }

    CEvent::~CEvent()
    {
        if (mEvent)
            CloseHandle(mEvent);
    }

};
#ifdef EXTERNAL_DISTRO
};
#endif


#endif  //  #if defined(_MT)

