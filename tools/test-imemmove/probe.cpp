#include "sharedFoundation/FirstSharedFoundation.h"
#include <stddef.h>
#include <stdio.h>

#ifdef TEST_BASELINE
#define INT_MOVE memmove
#else
#define INT_MOVE imemmove
#endif

typedef char WidthCheck[sizeof(void *) * 8 == EXPECT_BITS ? 1 : -1];
void *(*intMove)(void *, void const *, int) = &INT_MOVE;
void *(*sizeMove)(void *, void const *, size_t) = &memmove;

int main()
{
    int const starts[][3] = {{0, 3, 8}, {3, 0, 8}, {0, 12, 8}, {2, 2, 8}, {2, 4, 0}};
    int checks = 0;
    for (unsigned int c = 0; c < sizeof(starts)/sizeof(starts[0]); ++c)
        for (int kind = 0; kind != 3; ++kind)
        {
            unsigned char actual[32], expected[32], before[32];
            for (int i = 0; i != 32; ++i) actual[i] = expected[i] = before[i] = static_cast<unsigned char>(i + 17);
            int const dst = starts[c][0], src = starts[c][1], length = starts[c][2];
            for (int i = 0; i != length; ++i) expected[dst + i] = before[src + i];
            void *result;
            if (kind == 0) result = intMove(actual + dst, actual + src, length);
            else if (kind == 1) result = sizeMove(actual + dst, actual + src, static_cast<size_t>(length));
            else result = memmove(actual + dst, actual + src, static_cast<ptrdiff_t>(length));
            if (result != actual + dst) return 1;
            ++checks;
            if (memcmp(actual, expected, sizeof(actual)) != 0) return 2;
            ++checks;
        }
    printf("PASS %d valid overlap and overload checks\n", checks);
    return checks == 30 ? 0 : 3;
}
