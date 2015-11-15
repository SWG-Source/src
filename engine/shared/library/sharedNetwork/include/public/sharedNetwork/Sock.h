#if defined(PLATFORM_WIN32)
#include "../../src/win32/Sock.h"
#elif defined(PLATFORM_LINUX)
#include "../../src/linux/Sock.h"
#else
#error Unsupported platform
#endif

