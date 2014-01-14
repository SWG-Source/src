// ======================================================================
//
// PooledString.cpp
//
// Copyright 2003 Sony Online Entertainment
//
// ======================================================================

#include "sharedUtility/FirstSharedUtility.h"
#include "sharedUtility/PooledString.h"
#include "sharedFoundation/ExitChain.h"
#include "sharedFoundation/LessPointerComparator.h"

// ======================================================================

namespace PooledStringNamespace
{

// ----------------------------------------------------------------------

typedef std::map<std::string const *, int, LessPointerComparator> PooledStringMap;
PooledStringMap *s_pooledStringMap = 0;
bool s_installed = false;

// ----------------------------------------------------------------------

void removePooledString(std::string const &s)
{
	PooledStringMap::iterator i = s_pooledStringMap->find(&s);
	FATAL(i == s_pooledStringMap->end(), ("Bad refcounting in PooledString"));
	if (--((*i).second) == 0)
	{
		delete (*i).first;
		s_pooledStringMap->erase(i);
	}
}

// ----------------------------------------------------------------------

std::string const *addPooledString(std::string const &s)
{
	PooledStringMap::iterator i = s_pooledStringMap->find(&s);
	if (i != s_pooledStringMap->end())
	{
		++((*i).second);
		return (*i).first;
	}
	std::string const *newStr = new std::string(s);
	s_pooledStringMap->insert(std::make_pair(newStr, 1));
	return newStr;
}

// ----------------------------------------------------------------------

}
using namespace PooledStringNamespace;

// ======================================================================

void PooledString::install() // static
{
	DEBUG_FATAL(s_installed, ("PooledString::install already installed"));
	s_pooledStringMap = new PooledStringMap;
	ExitChain::add(PooledString::remove, "PooledString");
	s_installed = true;
}

// ----------------------------------------------------------------------

void PooledString::remove() // static
{
	DEBUG_FATAL(!s_installed, ("PooledString::remove not installed"));
	for (PooledStringMap::iterator i = s_pooledStringMap->begin(); i != s_pooledStringMap->end(); ++i)
		delete (*i).first;
	delete s_pooledStringMap;
	s_pooledStringMap = 0;
	s_installed = false;
}

// ----------------------------------------------------------------------

PooledString::PooledString() :
	m_string(addPooledString(std::string()))
{
}

// ----------------------------------------------------------------------

PooledString::PooledString(PooledString const &s) :
	m_string(addPooledString(*s.m_string))
{
}

// ----------------------------------------------------------------------

PooledString::PooledString(std::string const &s) :
	m_string(addPooledString(s))
{
}

// ----------------------------------------------------------------------

PooledString::PooledString(char const *s) :
	m_string(addPooledString(std::string(s)))
{
}

// ----------------------------------------------------------------------

PooledString &PooledString::operator=(PooledString const &s)
{
	if (m_string != s.m_string)
	{
		removePooledString(*m_string);
		m_string = addPooledString(*s.m_string);
	}
	return *this;
}

// ----------------------------------------------------------------------

PooledString &PooledString::operator=(std::string const &s)
{
	if (*m_string != s)
	{
		removePooledString(*m_string);
		m_string = addPooledString(s);
	}
	return *this;
}

// ----------------------------------------------------------------------

PooledString &PooledString::operator=(char const *s)
{
	if (*m_string != s)
	{
		removePooledString(*m_string);
		m_string = addPooledString(s);
	}
	return *this;
}

// ----------------------------------------------------------------------

PooledString::operator std::string const &() const
{
	return *m_string;
}

// ----------------------------------------------------------------------

bool PooledString::operator==(PooledString const &s) const
{
	return m_string == s.m_string;
}

// ----------------------------------------------------------------------

bool PooledString::operator!=(PooledString const &s) const
{
	return m_string != s.m_string;
}

// ----------------------------------------------------------------------

bool PooledString::operator<(PooledString const &s) const
{
	return (*m_string) < (*s.m_string);
}

// ----------------------------------------------------------------------

char const *PooledString::c_str() const
{
	return m_string->c_str();
}

// ======================================================================

namespace Archive
{

// ----------------------------------------------------------------------

void get(ReadIterator &source, PooledString &target)
{
	std::string s;
	get(source, s);
	target = s;
}

// ----------------------------------------------------------------------

void put(ByteStream &target, PooledString const &source)
{
	put(target, static_cast<std::string const &>(source));
}

// ----------------------------------------------------------------------

} // namespace Archive

// ======================================================================

