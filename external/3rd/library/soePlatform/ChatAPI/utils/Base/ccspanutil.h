#ifndef _CCSPANUTIL_H
#define _CCSPANUTIL_H

#pragma warning (disable : 4786)
#include <stdio.h>
#include <string>

#ifdef EXTERNAL_DISTRO
namespace NAMESPACE 
{

#endif
namespace Base 
{

    class CSpanUtil 
    {
    public:    
        
		//determine the type of credit card according to credit number
        static char *getDescriptionFromCardNum(const char *cardNum);

        // generates a credit card span for an account number string up to a length of any size.
        static std::string generateCCSpan(std::string cardNumber);

        // generartes credit card description consist of card type + a credit card span
        static std::string generateCCDescription(std::string cardNumber);

        // constructor
	    CSpanUtil();
        
        // destructor
	    ~CSpanUtil();        
    };    
};
#ifdef EXTERNAL_DISTRO
};
#endif

#endif
