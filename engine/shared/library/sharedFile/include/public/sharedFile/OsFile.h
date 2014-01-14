#if defined(PLATFORM_WIN32)
#include "../../src/win32/OsFile.h"
#elif defined(PLATFORM_LINUX)
#include "../../src/linux/OsFile.h"
#else
#error unsupported platform
#endif
