#ifndef INCLUDED_NetworkMessageTimestamp_H
#define INCLUDED_NetworkMessageTimestamp_H

#include <cstdint>
#include <ctime>
#include <limits>
#include <stdexcept>

namespace NetworkMessageTimestamp
{
// Legacy wire timestamps are signed 32-bit seconds. Host time_t may be wider.
inline int32_t fromTime(time_t value)
{
    static_assert(std::numeric_limits<time_t>::is_signed, "signed host time_t required");
    if (value < (std::numeric_limits<int32_t>::min)() ||
        value > (std::numeric_limits<int32_t>::max)())
        throw std::out_of_range("timestamp exceeds signed 32-bit wire range");
    return static_cast<int32_t>(value);
}
}

#endif
