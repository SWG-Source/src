// Standalone test process termination only. No serialization code is replaced.
// The full server Fatal implementation depends on crash-report/runtime setup.
#include <cstdarg>
#include <cstdio>
#include <cstdlib>
void Fatal(char const *format, ...)
{
    va_list args;
    va_start(args, format);
    std::vfprintf(stderr, format, args);
    va_end(args);
    std::abort();
}
