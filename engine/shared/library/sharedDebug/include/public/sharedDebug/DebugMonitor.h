#if defined(PLATFORM_WIN32)
#include "../../src/win32/DebugMonitor.h"
#elif defined(PLATFORM_LINUX)
#include "../../src/linux/DebugMonitor.h"
#else
#error unsupported platform
#endif
