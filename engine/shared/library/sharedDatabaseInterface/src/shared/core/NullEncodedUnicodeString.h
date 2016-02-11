// ======================================================================
//
// NullEncodedUnicodeString.h
// copyright (c) 2004 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_NullEncodedUnicodeString_H
#define INCLUDED_NullEncodedUnicodeString_H

// ======================================================================

#include "Unicode.h"

// ======================================================================

/**
 * A unicode-string-compatible class that encodes a nullptr value as a single space
 */
class NullEncodedUnicodeString
{
  public:
	NullEncodedUnicodeString();
	explicit NullEncodedUnicodeString(Unicode::String const & src);
	Unicode::String const & getValue() const;
	void setValue(Unicode::String const & src);
	Unicode::String const & getValueEncoded() const;
	void setValueEncoded(Unicode::String const & src);
	
  private:
	Unicode::String m_value;
};

// ======================================================================

namespace Archive
{
	class ReadIterator;
	class ByteStream;

	void get(ReadIterator & source, NullEncodedUnicodeString & target);
	void put(ByteStream & target, const NullEncodedUnicodeString & source);
}

// ======================================================================

#endif
