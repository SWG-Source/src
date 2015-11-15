#if defined(PLATFORM_WIN32)
#include "../../src/win32/OsNewDel.h"
#elif defined(PLATFORM_LINUX)
#include "../../src/linux/OsNewDel.h"
#else
#error unsupported platform
#endif
