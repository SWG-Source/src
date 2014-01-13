#if defined(PLATFORM_WIN32)
#include "../../src/win32/PerformanceTimer.h"
#elif defined(PLATFORM_LINUX)
#include "../../src/linux/PerformanceTimer.h"
#else
#error unsupported platform
#endif
