#if defined(PLATFORM_WIN32)
#include "../../src/win32/ProfilerTimer.h"
#elif defined(PLATFORM_LINUX)
#include "../../src/linux/ProfilerTimer.h"
#else
#error unsupported platform
#endif
