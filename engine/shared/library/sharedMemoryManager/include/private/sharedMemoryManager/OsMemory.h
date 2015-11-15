#if defined(PLATFORM_WIN32)
#include "../../src/win32/OsMemory.h"
#elif defined(PLATFORM_LINUX)
#include "../../src/linux/OsMemory.h"
#else
#error unsupported platform
#endif
