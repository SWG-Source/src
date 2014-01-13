// ======================================================================
//
// Binary.h
// jeff grills
//
// copyright 1998 Bootprint Entertainment
//
// ======================================================================

#ifndef BINARY_H
#define BINARY_H

// ======================================================================

//lint -save -e1923 // could become const variable

#define HEX_DIGIT_0000 0
#define HEX_DIGIT_0001 1
#define HEX_DIGIT_0010 2
#define HEX_DIGIT_0011 3
#define HEX_DIGIT_0100 4
#define HEX_DIGIT_0101 5
#define HEX_DIGIT_0110 6
#define HEX_DIGIT_0111 7
#define HEX_DIGIT_1000 8
#define HEX_DIGIT_1001 9
#define HEX_DIGIT_1010 a
#define HEX_DIGIT_1011 b
#define HEX_DIGIT_1100 c
#define HEX_DIGIT_1101 d
#define HEX_DIGIT_1110 e
#define HEX_DIGIT_1111 f

//line -restore

#define HEX_DIGIT(a)              HEX_DIGIT_ ## a

#define BINARY1H(a)               (0x ## a)
#define BINARY1I(a)               BINARY1H(a)
#define BINARY1(a)                BINARY1I(HEX_DIGIT(a))

#define BINARY2H(a,b)             (0x ## a ## b)
#define BINARY2I(a,b)             BINARY2H(a,b)
#define BINARY2(a,b)              BINARY2I(HEX_DIGIT(a), HEX_DIGIT(b))

#define BINARY3H(a,b,c)           (0x ## a ## b ## c)
#define BINARY3I(a,b,c)           BINARY3H(a,b,c)
#define BINARY3(a,b,c)            BINARY3I(HEX_DIGIT(a), HEX_DIGIT(b), HEX_DIGIT(c))

#define BINARY4H(a,b,c,d)         (0x ## a ## b ## c ## d)
#define BINARY4I(a,b,c,d)         BINARY4H(a,b,c,d)
#define BINARY4(a,b,c,d)          BINARY4I(HEX_DIGIT(a), HEX_DIGIT(b), HEX_DIGIT(c), HEX_DIGIT(d))

#define BINARY5H(a,b,c,d,e)       (0x ## a ## b ## c ## d ## e)
#define BINARY5I(a,b,c,d,e)       BINARY5H(a,b,c,d,e)
#define BINARY5(a,b,c,d,e)        BINARY5I(HEX_DIGIT(a), HEX_DIGIT(b), HEX_DIGIT(c), HEX_DIGIT(d), HEX_DIGIT(e))

#define BINARY6H(a,b,c,d,e,f)     (0x ## a ## b ## c ## d ## e ## f)
#define BINARY6I(a,b,c,d,e,f)     BINARY6H(a,b,c,d,e,f)
#define BINARY6(a,b,c,d,e,f)      BINARY6I(HEX_DIGIT(a), HEX_DIGIT(b), HEX_DIGIT(c), HEX_DIGIT(d), HEX_DIGIT(e), HEX_DIGIT(f))

#define BINARY7H(a,b,c,d,e,f,g)   (0x ## a ## b ## c ## d ## e ## f ## g)
#define BINARY7I(a,b,c,d,e,f,g)   BINARY7H(a,b,c,d,e,f,g)
#define BINARY7(a,b,c,d,e,f,g)    BINARY7I(HEX_DIGIT(a), HEX_DIGIT(b), HEX_DIGIT(c), HEX_DIGIT(d), HEX_DIGIT(e), HEX_DIGIT(f), HEX_DIGIT(g))

#define BINARY8H(a,b,c,d,e,f,g,h) (0x ## a ## b ## c ## d ## e ## f ## g ## h)
#define BINARY8I(a,b,c,d,e,f,g,h) BINARY8H(a,b,c,d,e,f,g,h)
#define BINARY8(a,b,c,d,e,f,g,h)  BINARY8I(HEX_DIGIT(a), HEX_DIGIT(b), HEX_DIGIT(c), HEX_DIGIT(d), HEX_DIGIT(e), HEX_DIGIT(f), HEX_DIGIT(g), HEX_DIGIT(h))

// ======================================================================

#endif
