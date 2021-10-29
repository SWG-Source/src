// iterhash.cpp - written and placed in the public domain by Wei Dai

#include "FirstCrypto.h"
#include "iterhash.h"

NAMESPACE_BEGIN(CryptoPP)

#ifdef WORD64_AVAILABLE
template class IteratedHashBase<word64>;
#endif

template class IteratedHashBase<word32>;

NAMESPACE_END
