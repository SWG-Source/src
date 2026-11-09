#ifndef INCLUDED_NetworkMessageTimestamp_H
#define INCLUDED_NetworkMessageTimestamp_H

#include <cstdint>
#include <ctime>
#include <limits>

namespace NetworkMessageTimestamp
{
// Legacy wire timestamps are signed 32-bit seconds. Host time_t may be wider.
// Values outside the signed 32-bit range wrap modulo 2^32, which is exactly
// what the 32-bit build (32-bit time_t) has always sent, so the bytes on the
// wire are unchanged. Calendar times after 2038-01-19 03:14:07 UTC therefore
// appear negative to the client, as they would on a 32-bit server.
inline int32_t fromTime(time_t value)
{
    static_assert(std::numeric_limits<time_t>::is_signed, "signed host time_t required");
    uint32_t const low = static_cast<uint32_t>(static_cast<uint64_t>(static_cast<int64_t>(value)));
    return (low <= static_cast<uint32_t>((std::numeric_limits<int32_t>::max)()))
        ? static_cast<int32_t>(low)
        : static_cast<int32_t>(static_cast<int64_t>(low) - (static_cast<int64_t>(1) << 32));
}
}

#endif
