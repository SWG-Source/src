// ======================================================================
//
// FormatAttribute.h
//
// Marks a printf-style function so the compiler checks every call's
// arguments against its format string (GCC and Clang).
//
// ======================================================================

#ifndef INCLUDED_FormatAttribute_H
#define INCLUDED_FormatAttribute_H

#if defined(__GNUC__)
#define SWG_FORMAT_PRINTF(formatIndex, firstArgIndex) __attribute__((format(printf, formatIndex, firstArgIndex)))
#else
#define SWG_FORMAT_PRINTF(formatIndex, firstArgIndex)
#endif

#endif
