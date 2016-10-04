// ======================================================================
// 
// FormattedString.h
// Copyright 2004 Sony Online Entertainment, Inc.
//
// ======================================================================

#ifndef INCLUDED_FormattedString_H
#define INCLUDED_FormattedString_H

#include <cstdio>

// ----------------------------------------------------------------------
template <int bufferSize>
class FormattedString
{
public:

	FormattedString();

	char const * sprintf(char const * format, ...);
	char const * vsprintf(char const * format, va_list const & va);

private:

	char m_text[bufferSize];

	// Disabled

	FormattedString(FormattedString const &);
	FormattedString & operator =(FormattedString const &);
};

// ----------------------------------------------------------------------
template <int bufferSize>
inline FormattedString<bufferSize>::FormattedString()
{
	m_text[0] = '\0';
}

//-----------------------------------------------------------------------------
template <int bufferSize>
inline char const * FormattedString<bufferSize>::sprintf(char const * const format, ...)
{
	char const * result = nullptr;
	va_list va;

	va_start(va, format);
	{
		result = vsprintf(format, va);
	}
	va_end(va);

	return result;
}

// ----------------------------------------------------------------------
template <int bufferSize>
inline char const * FormattedString<bufferSize>::vsprintf(char const * const format, va_list const & va)
{
	// Format the string

	int const lastIndex = sizeof(m_text) - 1;
	int const charactersWritten = vsnprintf(m_text, lastIndex, format, va);

    // vsnprintf returns the number of characters written, not including
	// the terminating nullptr character, or a negative value if an output error occurs.
	// If the number of characters to write exceeds count, then count characters are
	// written and -1 is returned.

	// Handle overflow

	if (   (charactersWritten == lastIndex)
	    || (charactersWritten == -1))
	{
		m_text[lastIndex - 1] = '+';
		m_text[lastIndex] = '\0';
	}

	return m_text;
}

#endif // INCLUDED_FormattedString_H

// ======================================================================
