#if defined(PLATFORM_WIN32)
#include "../../src/win32/SQLC_Defs.h"
#elif defined(PLATFORM_LINUX)
#include "../../src/linux/SQLC_Defs.h"
#else
#error unsupported platform
#endif
