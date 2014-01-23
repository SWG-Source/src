//#include "FirstGame.h"
//#include <hash_map> //without this we get an internal compiler error

// This resolves a problem with stlport compiling under gcc 2.91 that ships with RedHat 6.2.
// Basically, the compiler has trouble instantiating an instance of _Stl_prime<bool> without it
// being explicitly defined.  So we have to instantiate one.
// We should try removing this file when either we a) upgrade the compiler or b) upgrate the OS.

//#define __stl_num_primes 28
//#define __PRIME_LIST_BODY { \
//  53ul,         97ul,         193ul,       389ul,       769ul,      \
//  1543ul,       3079ul,       6151ul,      12289ul,     24593ul,    \
//  49157ul,      98317ul,      196613ul,    393241ul,    786433ul,   \
//  1572869ul,    3145739ul,    6291469ul,   12582917ul,  25165843ul, \
//  50331653ul,   100663319ul,  201326611ul, 402653189ul, 805306457ul,\
//  1610612741ul, 3221225473ul, 4294967291ul  \
//}
//
//template <> const size_t std::_Stl_prime<bool>::_M_list[__stl_num_primes] = __PRIME_LIST_BODY;
//
//#undef __stl_num_primes
//#undef __PRIME_LIST_BODY
//