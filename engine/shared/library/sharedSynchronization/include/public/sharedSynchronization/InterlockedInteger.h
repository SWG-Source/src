#if defined(PLATFORM_WIN32)
#include "../../src/win32/InterlockedInteger.h"
#elif defined(PLATFORM_LINUX)
#include "../../src/linux/InterlockedInteger.h"
#else
#error unsupported platform
#endif
