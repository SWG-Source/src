#include "pid.h"

#ifdef WIN32
#include <Winsock2.h>
#else
#include <errno.h>
extern int errno;
#endif

#include <cstdio>
#include <cstring>

#define NAME_SIZE 256

#ifdef WIN32
std::string soegethostname()
{
	std::string hn = "";
	WORD wVersionRequested;
	WSADATA wsaData;
	char name[NAME_SIZE];
	wVersionRequested = MAKEWORD( 2, 0 );

	if ( WSAStartup( wVersionRequested, &wsaData ) == 0 )
	{

		if( gethostname ( name, sizeof(name)) != 0)
		{
			int err = WSAGetLastError();
			printf("gethostname error: %d\n", err);
		}

		WSACleanup( );
	}

	hn = name;
	return hn;
}

#else 

std::string soegethostname()
{
	char res[NAME_SIZE];
	memset(res, 0, NAME_SIZE);
	std::string retVal;

	retVal = res;
	return retVal;
}

#endif
