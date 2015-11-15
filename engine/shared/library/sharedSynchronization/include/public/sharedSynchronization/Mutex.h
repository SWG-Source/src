#if defined(PLATFORM_WIN32)
#include "../../src/win32/Mutex.h"
#elif defined(PLATFORM_LINUX)
#include "../../src/linux/Mutex.h"
#else
#error unsupported platform
#endif
