// ======================================================================
//
// NullEncodedUnicodeString.cpp
// copyright (c) 2004 Sony Online Entertainment
//
// ======================================================================

#include "sharedDatabaseInterface/FirstSharedDatabaseInterface.h"
#include "sharedDatabaseInterface/NullEncodedUnicodeString.h"

#include "Archive/Archive.h"
#include "Archive/ByteStream.h"
#include "UnicodeUtils.h"
#include "unicodeArchive/UnicodeArchive.h"

// ======================================================================

namespace NullEncodedUnicodeStringNamespace
{
	Unicode::String s_nullString;
	Unicode::String s_encodedNull(Unicode::narrowToWide(" "));
}
using namespace NullEncodedUnicodeStringNamespace;

// ======================================================================

NullEncodedUnicodeString::NullEncodedUnicodeString(Unicode::String const & src) :
		m_value(src.empty() ? s_encodedNull : src)
{
}

// ----------------------------------------------------------------------

NullEncodedUnicodeString::	NullEncodedUnicodeString() :
		m_value()
{
}

// ----------------------------------------------------------------------

void NullEncodedUnicodeString::setValue(Unicode::String const & src)
{
	m_value = src.empty() ? s_encodedNull : src;
}

// ----------------------------------------------------------------------

Unicode::String const & NullEncodedUnicodeString::getValue() const
{
	if (m_value == s_encodedNull)
		return s_nullString;
	else
		return m_value;
}

// ----------------------------------------------------------------------

Unicode::String const & NullEncodedUnicodeString::getValueEncoded() const
{
	return m_value;
}

// ----------------------------------------------------------------------

void NullEncodedUnicodeString::setValueEncoded(Unicode::String const & src)
{
	m_value = src;
}

// ======================================================================

namespace Archive
{
	void get(ReadIterator & source, NullEncodedUnicodeString & target)
	{
		Unicode::String temp;
		get(source, temp);
		target.setValue(temp);
	}

	// ----------------------------------------------------------------------
	
	void put(ByteStream & target, const NullEncodedUnicodeString & source)
	{
		put(target, source.getValue());
	}
}

// ======================================================================
