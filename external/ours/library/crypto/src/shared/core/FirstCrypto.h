#ifndef INCLUDED_FirstCrypto_H
#define INCLUDED_FirstCrypto_H

// C4018 signed/unsigned mismatch
// C4100 unreferenced formal parameter
// C4244 conversion from, possible loss of data
// C4511 copy constructor could not be generated
// C4512 assignment operator could not be generated
// C4514 unreferenced inline/local function has been removed
// C4663 C++ language change: to explicitly specialize class template 'codecvt' use the following syntax:
// C4290 C++ Exception Specification ignored
// C4505 unreferenced local function has been removed
// C4702 unreachable code

#pragma warning(disable: 4018 4100 4244 4511 4512 4514 4663 4290 4505 4702)

#if defined(_MSC_VER) && defined(_WIN64)
// STLport intentionally switches to pack(8) inside these balanced vendor includes.
#pragma warning(push)
#pragma warning(disable: 4103)
#endif

#include "config.h"
#include "cryptlib.h"
#include "misc.h"
#if defined(_MSC_VER) && defined(_WIN64)
#include <deque>
#include <memory>
#pragma warning(pop)
#endif

#endif
