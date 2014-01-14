#if defined(PLATFORM_WIN32)
#include "../../src/win32/Semaphore.h"
#elif defined(PLATFORM_LINUX)
#include "../../src/linux/Semaphore.h"
#else
#error unsupported platform
#endif
