// ======================================================================
//
// NullEncodedStandardString.cpp
// copyright (c) 2004 Sony Online Entertainment
//
// ======================================================================

#include "sharedDatabaseInterface/FirstSharedDatabaseInterface.h"
#include "sharedDatabaseInterface/NullEncodedStandardString.h"

#include "Archive/Archive.h"
#include "Archive/ByteStream.h"
#include "UnicodeUtils.h"
#include "unicodeArchive/UnicodeArchive.h"

// ======================================================================

namespace NullEncodedStandardStringNamespace
{
	std::string s_nullString;
	std::string s_encodedNull( " " );  // blank space
}
using namespace NullEncodedStandardStringNamespace;

// ======================================================================

NullEncodedStandardString::NullEncodedStandardString(std::string const & src) :
		m_value(src.empty() ? s_encodedNull : src)
{
}

// ----------------------------------------------------------------------

NullEncodedStandardString::	NullEncodedStandardString() :
		m_value()
{
}

// ----------------------------------------------------------------------

void NullEncodedStandardString::setValue(std::string const & src)
{
	m_value = src.empty() ? s_encodedNull : src;
}

// ----------------------------------------------------------------------

std::string const & NullEncodedStandardString::getValue() const
{
	if (m_value == s_encodedNull)
		return s_nullString;
	else
		return m_value;
}

// ----------------------------------------------------------------------

std::string const & NullEncodedStandardString::getValueEncoded() const
{
	return m_value;
}

// ----------------------------------------------------------------------

void NullEncodedStandardString::setValueEncoded(std::string const & src)
{
	m_value = src;
}

// ======================================================================

namespace Archive
{
	void get(ReadIterator & source, NullEncodedStandardString & target)
	{
		std::string temp;
		get(source, temp);
		target.setValue(temp);
	}

	// ----------------------------------------------------------------------
	
	void put(ByteStream & target, const NullEncodedStandardString & source)
	{
		put(target, source.getValue());
	}
}

// ======================================================================
