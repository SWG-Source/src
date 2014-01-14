// ============================================================================
//
// TextIterator.h
// Copyright Sony Online Entertainment
//
// ============================================================================

#ifndef INCLUDED_TextIterator_H
#define INCLUDED_TextIterator_H

#include "sharedMath/PackedRgb.h"

// ----------------------------------------------------------------------------
class TextIterator
{
public:

	TextIterator(Unicode::String const &text);

	void                   setRawText(Unicode::String const &text);

	Unicode::String const &getRawText() const;
	Unicode::String        getPrintableText();

	void                   insertTextAtFirstPrintablePosition(Unicode::String const &text);
	void                   insertTextAtPrintablePosition(int const position, Unicode::String const &text);
	void                   insertCurrentColorCodeAtPrintablePosition(int const position);
	void                   appendText(Unicode::String const &text);

private:

	void                   skipUnPrintable();

	Unicode::String                 m_rawText;
	Unicode::String                 m_currentColorCode;
	int                             m_rawIndex;
	Unicode::String::const_iterator m_iterPrintableIndex;

	// Disabled

	TextIterator();
	TextIterator &operator =(TextIterator const &);
};

// ============================================================================

#endif // INCLUDED_TextIterator_H
