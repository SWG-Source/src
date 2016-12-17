#include <stdio.h>
#include <stdint.h>

//-------------------------------------------------------------//
// "Malware related compile-time hacks with C++11" by LeFF	 //
// You can use this code however you like, I just don't really //
// give a shit, but if you feel some respect for me, please	//
// don't cut off this comment when copy-pasting... ;-)		 //
//-------------------------------------------------------------//

#ifndef vxCPLSEED
// If you don't specify the seed for algorithms, the time when compilation
// started will be used, seed actually changes the results of algorithms...
#define vxCPLSEED ((__TIME__[7] - '0') * 1    + (__TIME__[6] - '0') * 10  + \
                       (__TIME__[4] - '0') * 60   + (__TIME__[3] - '0') * 600 + \
                       (__TIME__[1] - '0') * 3600 + (__TIME__[0] - '0') * 36000)
#endif

// The constantify template is used to make sure that the result of constexpr
// function will be computed at compile-time instead of run-time
template<uint32_t Const> struct vxCplConstantify { enum { Value = Const }; };

// Compile-time mod of a linear congruential pseudorandom number generator,
// the actual algorithm was taken from "Numerical Recipes" book
constexpr uint32_t vxCplRandom(uint32_t Id) {
    return (1013904223 + 1664525 * ((Id > 0) ? (vxCplRandom(Id - 1)) : (vxCPLSEED))) & 0xFFFFFFFF;
}

// Compile-time random macros, can be used to randomize execution
// path for separate builds, or compile-time trash code generation
#define vxRANDOM(Min, Max) (Min + (vxRAND() % (Max - Min + 1)))
#define vxRAND()           (vxCplConstantify<vxCplRandom(__COUNTER__ + 1)>::Value)

// Compile-time recursive mod of string hashing algorithm,
// the actual algorithm was taken from Qt library (this
// function isn't case sensitive due to vxCplTolower)
constexpr char vxCplTolower(char Ch) { return (Ch >= 'A' && Ch <= 'Z') ? (Ch - 'A' + 'a') : (Ch); }

constexpr uint32_t vxCplHashPart3(char Ch, uint32_t Hash) { return ((Hash << 4) + vxCplTolower(Ch)); }

constexpr uint32_t vxCplHashPart2(char Ch, uint32_t Hash) {
    return (vxCplHashPart3(Ch, Hash) ^ ((vxCplHashPart3(Ch, Hash) & 0xF0000000) >> 23));
}

constexpr uint32_t vxCplHashPart1(char Ch, uint32_t Hash) { return (vxCplHashPart2(Ch, Hash) & 0x0FFFFFFF); }

constexpr uint32_t vxCplHash(const char *Str) { return (*Str) ? (vxCplHashPart1(*Str, vxCplHash(Str + 1))) : (0); }

// Compile-time hashing macro, hash values changes using the first pseudorandom number in sequence
#define vxHASH(Str) (uint32_t)(vxCplConstantify<vxCplHash(Str)>::Value ^ vxCplConstantify<vxCplRandom(1)>::Value)

// Compile-time generator for list of indexes (0, 1, 2, ...)
template<uint32_t...> struct vxCplIndexList {};
template<typename IndexList, uint32_t Right> struct vxCplAppend;
template<uint32_t... Left, uint32_t Right>
    struct vxCplAppend<vxCplIndexList<Left...>, Right> { typedef vxCplIndexList<Left..., Right> Result; };
template<uint32_t N>
    struct vxCplIndexes { typedef typename vxCplAppend<typename vxCplIndexes<N - 1>::Result, N - 1>::Result Result; };
template<> struct vxCplIndexes<0> { typedef vxCplIndexList<> Result; };

// Compile-time string encryption of a single character
const int vxCplEncryptCharKey = vxRANDOM(0, 0xFF);

constexpr char vxCplEncryptChar(const char Ch, uint32_t Idx) { return Ch ^ (vxCplEncryptCharKey + Idx); }

// Compile-time string encryption class
template<typename IndexList> struct vxCplEncryptedString;

template<uint32_t... Idx> struct vxCplEncryptedString<vxCplIndexList<Idx...> > {
    char Value[sizeof...(Idx) + 1]; // Buffer for a string

    // Compile-time constructor
    constexpr inline vxCplEncryptedString(const char *const Str) : Value{vxCplEncryptChar(Str[Idx], Idx)...} {}

    // Run-time decryption
    char *decrypt() {
        for (volatile uint32_t t = 0; t < sizeof...(Idx); t++) {
            this->Value[t] = this->Value[t] ^ (vxCplEncryptCharKey + t);
        }
        this->Value[sizeof...(Idx)] = '\0';
        return this->Value;
    }
};

// Compile-time string encryption macro
#define vxENCRYPT(Str) (vxCplEncryptedString<vxCplIndexes<sizeof(Str) - 1>::Result>(Str))
