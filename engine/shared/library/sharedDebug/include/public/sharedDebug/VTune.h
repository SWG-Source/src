#if defined(PLATFORM_WIN32)
#include "../../src/win32/VTune.h"
#elif defined(PLATFORM_LINUX)
#include "../../src/linux/VTune.h"
#else
#error unsupported platform
#endif
