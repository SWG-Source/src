#if defined(PLATFORM_WIN32)
#include "../../src/win32/RegexServices.h"
#elif defined(PLATFORM_LINUX)
#include "../../src/linux/RegexServices.h"
#else
#error unsupported platform
#endif
