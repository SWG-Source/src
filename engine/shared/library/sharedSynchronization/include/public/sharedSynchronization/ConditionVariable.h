#if defined(PLATFORM_WIN32)
#include "../../src/win32/ConditionVariable.h"
#elif defined(PLATFORM_LINUX)
#include "../../src/linux/ConditionVariable.h"
#else
#error unsupported platform
#endif
