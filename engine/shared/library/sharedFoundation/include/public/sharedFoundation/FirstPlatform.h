#if defined(PLATFORM_WIN32)
#include "../../src/win32/FirstPlatform.h"
#elif defined(PLATFORM_LINUX)
#include "../../src/linux/FirstPlatform.h"
#else
#error unsupported platform
#endif
