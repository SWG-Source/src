// File.h: interface for the CFile class.
//
//////////////////////////////////////////////////////////////////////

#ifndef BASE_FILE_H
#define BASE_FILE_H

#include <stdio.h>

class CFile  
{
    public:
	    CFile(const char *);
	    virtual ~CFile();

        bool        IsOpen();

    private:
        FILE* mFileHandle;
};

#endif // BASE_FILE_H
