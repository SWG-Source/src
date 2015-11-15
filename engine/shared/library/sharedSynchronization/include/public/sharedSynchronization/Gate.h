#if defined(PLATFORM_WIN32)
#include "../../src/win32/Gate.h"
#elif defined(PLATFORM_LINUX)
#include "../../src/linux/Gate.h"
#else
#error unsupported platform
#endif
