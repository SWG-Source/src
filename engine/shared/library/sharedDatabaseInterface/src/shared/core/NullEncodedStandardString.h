// ======================================================================
//
// NullEncodedStandardString.h
// copyright (c) 2004 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_NullEncodedStandardString_H
#define INCLUDED_NullEncodedStandardString_H

// ======================================================================

#include "Unicode.h"

// ======================================================================

/**
 * A unicode-string-compatible class that encodes a nullptr value as a single space
 */
class NullEncodedStandardString
{
  public:
	NullEncodedStandardString();
	explicit NullEncodedStandardString(std::string const & src);
	std::string const & getValue() const;
	void setValue(std::string const & src);
	std::string const & getValueEncoded() const;
	void setValueEncoded(std::string const & src);
	
  private:
	std::string m_value;
};

// ======================================================================

namespace Archive
{
	class ReadIterator;
	class ByteStream;

	void get(ReadIterator & source, NullEncodedStandardString & target);
	void put(ByteStream & target, const NullEncodedStandardString & source);
}

// ======================================================================

#endif // INCLUDED_NullEncodedStandardString_H
