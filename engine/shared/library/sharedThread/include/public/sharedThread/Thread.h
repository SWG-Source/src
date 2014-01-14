#if defined(PLATFORM_WIN32)
#include "../../src/win32/Thread.h"
#elif defined(PLATFORM_LINUX)
#include "../../src/linux/Thread.h"
#else
#error unsupported platform
#endif
