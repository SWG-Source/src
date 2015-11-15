// ======================================================================
//
// vsnprintf.h
// jeff grills
//
// copyright 1998 Bootprint Entertainment
//
// ======================================================================

#ifndef VSNPRINTF_H
#define VSNPRINTF_H

// ======================================================================

#ifdef _MSC_VER

int vsnprintf(char *buffer, size_t count, const char *format, va_list va);

#endif

// ======================================================================

#endif

