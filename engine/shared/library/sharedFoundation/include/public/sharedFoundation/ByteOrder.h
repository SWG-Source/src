#if defined(PLATFORM_WIN32)
#include "../../src/win32/ByteOrder.h"
#elif defined(PLATFORM_LINUX)
#include "../../src/linux/ByteOrder.h"
#else
#error unsupported platform
#endif
