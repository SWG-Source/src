#if defined(PLATFORM_WIN32)
#include "../../src/win32/PerThreadData.h"
#elif defined(PLATFORM_LINUX)
#include "../../src/linux/PerThreadData.h"
#else
#error unsupported platform
#endif
