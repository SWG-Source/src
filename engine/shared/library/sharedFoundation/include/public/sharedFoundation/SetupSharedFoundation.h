#if defined(PLATFORM_WIN32)
#include "../../src/win32/SetupSharedFoundation.h"
#elif defined(PLATFORM_LINUX)
#include "../../src/linux/SetupSharedFoundation.h"
#else
#error unsupported platform
#endif
