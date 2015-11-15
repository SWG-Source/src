#if defined(PLATFORM_WIN32)
#include "../../src/win32/ConfigSharedFoundation.h"
#elif defined(PLATFORM_LINUX)
#include "../../src/linux/ConfigSharedFoundation.h"
#else
#error unsupported platform
#endif
