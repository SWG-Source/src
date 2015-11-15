#if defined(PLATFORM_WIN32)
#include "../../src/win32/PlatformGlue.h"
#elif defined(PLATFORM_LINUX)
#include "../../src/linux/PlatformGlue.h"
#else
#error unsupported platform
#endif
