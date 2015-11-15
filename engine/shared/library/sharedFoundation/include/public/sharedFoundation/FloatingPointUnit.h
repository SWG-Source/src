#if defined(PLATFORM_WIN32)
#include "../../src/win32/FloatingPointUnit.h"
#elif defined(PLATFORM_LINUX)
#include "../../src/linux/FloatingPointUnit.h"
#else
#error unsupported platform
#endif
