#include "ChatEnum.h"
#include <iostream>

namespace ChatSystem {

using namespace std;
using namespace Plat_Unicode;

Plat_Unicode::String ChatUnicodeString::getEmptyString()
{
	return narrowToWide("");
}

ChatUnicodeString::ChatUnicodeString() 
{
	m_wideString = getEmptyString();
	m_cString = wideToNarrow(m_wideString);
}

ChatUnicodeString::ChatUnicodeString(const unsigned short *data, unsigned length)
{ 
	m_wideString.assign(data, length);
	m_cString = wideToNarrow(m_wideString);
}

ChatUnicodeString::ChatUnicodeString(const char *nData, unsigned length)
{
	m_cString.assign(nData, length);
	m_wideString = narrowToWide(m_cString);
}

ChatUnicodeString::ChatUnicodeString(const Plat_Unicode::String& src)
{
	m_wideString = src;
	m_cString = wideToNarrow(m_wideString);
}

ChatUnicodeString::ChatUnicodeString(const std::string& nSrc)
{
	m_cString = nSrc;
	m_wideString = narrowToWide(m_cString);
}

ChatUnicodeString::ChatUnicodeString(const char *nStrSrc)
{
	if (nStrSrc==nullptr) 
	{
		m_wideString = getEmptyString();
		m_cString = wideToNarrow(m_wideString);
	}
	else  
	{
		m_cString.assign(nStrSrc);
		m_wideString = narrowToWide(m_cString);
	}
}

ChatUnicodeString::ChatUnicodeString(const ChatUnicodeString& src)
{
	m_wideString = src.m_wideString;
	m_cString = wideToNarrow(m_wideString);
}

// = operator
ChatUnicodeString& ChatUnicodeString::operator=(const Plat_Unicode::String& src)
{
	m_wideString = src;
	m_cString = wideToNarrow(m_wideString);

	return *this;
}

ChatUnicodeString& ChatUnicodeString::operator=(const std::string& nSrc)
{
	m_cString = nSrc;
	m_wideString = narrowToWide(m_cString);

	return *this;
}

ChatUnicodeString& ChatUnicodeString::operator=(const char * nStrSrc)
{
	m_cString = nStrSrc;
	m_wideString = narrowToWide(m_cString);

	return *this;
}

ChatUnicodeString& ChatUnicodeString::operator=(const ChatUnicodeString& src)
{
	if (this != &src) {
		m_wideString = src.m_wideString;
		m_cString = wideToNarrow(m_wideString);
	}

	return *this;
}

// += operator
ChatUnicodeString& ChatUnicodeString::operator+=(const ChatUnicodeString& src)
{
	m_wideString += src.m_wideString;
	m_cString = wideToNarrow(m_wideString);

    return *this;
}

// + operator
ChatUnicodeString ChatUnicodeString::operator+(const ChatUnicodeString& src)
{
	ChatUnicodeString tmp;

	tmp.m_wideString = m_wideString + src.m_wideString;
	m_cString = wideToNarrow(m_wideString);

	return tmp;
}

const char *ChatUnicodeString::c_str() const {
	return m_cString.c_str(); 
}

AvatarSnoopPair::AvatarSnoopPair(const Plat_Unicode::String &name, const Plat_Unicode::String &address)
: m_name(name),
  m_address(address)
{
}
	
};
