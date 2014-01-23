// ======================================================================
//
// BufferString.cpp
// copyright (c) 2004 Sony Online Entertainment
//
// ======================================================================

#include "sharedDatabaseInterface/FirstSharedDatabaseInterface.h"
#include "sharedDatabaseInterface/BufferString.h"

#include "UnicodeUtils.h"
#include "sharedDatabaseInterface/DbServer.h"
#include "sharedDatabaseInterface/NullEncodedStandardString.h"
#include "sharedDatabaseInterface/NullEncodedUnicodeString.h"
#include "sharedLog/Log.h"

#include <climits>

using namespace DB;

// ======================================================================

BufferString::BufferString(size_t maxSize) :
		Bindable(),
		m_value(),
		m_maxSize(maxSize)
{
}

// ----------------------------------------------------------------------

void BufferString::getValue(std::string & value) const
{
	if (isNull())
		value.clear();
	else
		value=m_value;
}

// ----------------------------------------------------------------------

void BufferString::getValue(Unicode::String & value) const
{
	if (isNull())
		value.clear();
	else
		value=Unicode::utf8ToWide(m_value);
}

// ----------------------------------------------------------------------

Unicode::String BufferString::getValue() const
{
	static const Unicode::String empty;
	
	if (isNull())
		return empty;
	else
		return Unicode::utf8ToWide(m_value);
}

// ----------------------------------------------------------------------

BufferString & BufferString::operator=(const std::string & value)
{
	setValue(value);
	return *this;
}

// ----------------------------------------------------------------------

BufferString & BufferString::operator=(const Unicode::String & value)
{
	setValue(value);
	return *this;
}

// ----------------------------------------------------------------------

void BufferString::setValue(const std::string & value)
{
	if (value.size() > m_maxSize)
	{
		if (DB::Server::getFatalOnDataError())
		{
			FATAL(true,("DatabaseError:  Attempt to save too long a string to the database:  \"%s\"",value.c_str()));
		}
		else
		{
			WARNING_STACK_DEPTH(true,(INT_MAX,"DatabaseError:  Attempt to save too long a string to the database.  (Text is in the log output.)"));
			LogManager::logLongText("DatabaseError",std::string("String from previous error is \"")+value+"\"");
			m_value=std::string(value,0,m_maxSize);
		}
	}
	else
		m_value=value;
	
	indicator=m_value.size();
}

// ----------------------------------------------------------------------

void BufferString::setValue(const Unicode::String & value)
{
	setValue(Unicode::wideToUTF8(value));
}

// ----------------------------------------------------------------------

const std::string & BufferString::getValueASCII() const
{
	static const std::string empty;
	
	if (isNull())
		return empty;
	else
		return m_value;
}

// ----------------------------------------------------------------------

BufferString & BufferString::operator=(const NullEncodedUnicodeString & value)
{
	return operator=(value.getValueEncoded());
}

// ----------------------------------------------------------------------

void BufferString::getValue(NullEncodedUnicodeString & value) const
{
	Unicode::String temp;
	getValue(temp);
	value.setValueEncoded(temp);
}

// ----------------------------------------------------------------------

BufferString & BufferString::operator=(const NullEncodedStandardString & value)
{
	return operator=(value.getValueEncoded());
}

// ----------------------------------------------------------------------

void BufferString::getValue(NullEncodedStandardString & value) const
{
	std::string temp;
	getValue(temp);
	value.setValueEncoded(temp);
}

// ----------------------------------------------------------------------

std::string BufferString::outputValue() const
{
	return m_value;
}

// ======================================================================
