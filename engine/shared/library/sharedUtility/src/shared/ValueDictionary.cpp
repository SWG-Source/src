// ======================================================================
//
// ValueDictionary.cpp
// Copyright 2004 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#include "sharedUtility/FirstSharedUtility.h"
#include "sharedUtility/ValueDictionary.h"

#include "sharedUtility/ValueTypeBase.h"

// ======================================================================

ValueDictionary::ValueDictionary() :
	m_valueMap()
{
}

// ----------------------------------------------------------------------

ValueDictionary::ValueDictionary(ValueDictionary const & dictionary) :
	m_valueMap()
{
	*this = dictionary;
}

// ----------------------------------------------------------------------

ValueDictionary::~ValueDictionary()
{
	clear();
}

// ----------------------------------------------------------------------

ValueDictionary & ValueDictionary::operator =(ValueDictionary const & rhs)
{
	if (this != &rhs)
	{
		clear();

		for (DictionaryValueMap::const_iterator iter = rhs.m_valueMap.begin(); iter != rhs.m_valueMap.end(); ++iter)
		{
			NOT_NULL(iter->second);
			m_valueMap[iter->first] = iter->second->clone();
		}
	}

	return *this;
}

// ----------------------------------------------------------------------

void ValueDictionary::insert(std::string const & name, ValueTypeBase const & value)
{
	DictionaryValueMap::iterator iter = m_valueMap.find(name);
	if (iter != m_valueMap.end())
	{
		delete iter->second;
		iter->second = value.clone();
	}
	else
	{
		m_valueMap[name] = value.clone();
	}
}

// ----------------------------------------------------------------------

void ValueDictionary::insert(ValueDictionary const & dictionary)
{
	for (DictionaryValueMap::const_iterator iter = dictionary.m_valueMap.begin(); iter != dictionary.m_valueMap.end(); ++iter)
	{
		NOT_NULL(iter->second);
		insert(iter->first, *(iter->second));
	}
}

// ----------------------------------------------------------------------

void ValueDictionary::remove(std::string const & name)
{
	DictionaryValueMap::iterator iter = m_valueMap.find(name);
	if (iter != m_valueMap.end())
	{
		delete iter->second;
		m_valueMap.erase(iter);
	}
}

// ----------------------------------------------------------------------

bool ValueDictionary::exists(std::string const & name) const
{
	DictionaryValueMap::const_iterator iter = m_valueMap.find(name);

	return (iter != m_valueMap.end());
}

// ----------------------------------------------------------------------

ValueTypeBase * ValueDictionary::getCopy(std::string const & name) const
{
	ValueTypeBase * returnValue = nullptr;

	DictionaryValueMap::const_iterator iter = m_valueMap.find(name);
	if (iter != m_valueMap.end())
	{
		NOT_NULL(iter->second);
		returnValue = iter->second->clone(); // caller is responsible for freeing returned object
	}

	return returnValue;
}

// ----------------------------------------------------------------------

void ValueDictionary::clear()
{
	for (DictionaryValueMap::const_iterator iter = m_valueMap.begin(); iter != m_valueMap.end(); ++iter)
		delete iter->second;

	m_valueMap.clear();
}

// ======================================================================
