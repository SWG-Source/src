#ifndef INCLUDED_ArchiveCount_H
#define INCLUDED_ArchiveCount_H

#include <cstddef>
#include <cstdint>
#include <limits>
#include <stdexcept>

namespace ArchiveCount
{
// Legacy wire counts and lengths are 32-bit. Host sizes may be wider.
// Count is the field's legacy type and keeps its signedness.
template <typename Count>
inline Count fromSize(std::size_t value)
{
    static_assert(std::numeric_limits<Count>::is_integer && sizeof(Count) == 4, "32-bit wire count required");
    if (value > static_cast<std::size_t>((std::numeric_limits<Count>::max)()))
        throw std::out_of_range("container size exceeds 32-bit wire count range");
    return static_cast<Count>(value);
}
}

#endif
