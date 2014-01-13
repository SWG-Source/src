// ======================================================================
//
// MacroFoundation.h
// copyright (c) 1998 Bootprint Entertainment
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_MacroFoundation_H
#define INCLUDED_MacroFoundation_H

// ======================================================================

// No-operation
#define NOP              (static_cast<void>(0))   //lint !e1923 //could become const variable

// To prevent "Unreference" compiler warning
#define UNREF(a)         (static_cast<void>(a))

// To prevent "Ignoring return value" lint warnings
#define IGNORE_RETURN(a) (static_cast<void>(a))

// int version of the sizeof operator
#define isizeof(a) static_cast<int>(sizeof(a))

// ======================================================================

#endif
