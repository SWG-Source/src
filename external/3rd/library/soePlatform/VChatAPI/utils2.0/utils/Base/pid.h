#ifndef SOE_PID_H
#define SOE_PID_H

#ifndef WIN32
#include <sys/types.h>
#include <unistd.h>
#define soegetpid() (long)getpid()
#endif

#ifdef WIN32
// include winsock 2 before windows to not have winsock 1 included . . .
#include <WinSock2.h>
#include <winbase.h>

#define soegetpid() (long)GetCurrentProcessId()

#endif

#include <string>

std::string soegethostname();

#endif
