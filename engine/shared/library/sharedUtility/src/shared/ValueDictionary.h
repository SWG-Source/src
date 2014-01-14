// ======================================================================
//
// ValueDictionary.h
// Copyright 2004 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#ifndef INCLUDED_ValueDictionary_H
#define INCLUDED_ValueDictionary_H

// ======================================================================

#include <map>
#include <string>

// ======================================================================

class ValueTypeBase;

// ======================================================================

typedef std::map<std::string, ValueTypeBase *> DictionaryValueMap;

// ======================================================================

class ValueDictionary
{
public:

	ValueDictionary();
	ValueDictionary(ValueDictionary const & dictionary);

	virtual ~ValueDictionary();

public:

	ValueDictionary &operator =(ValueDictionary const & rhs);

	void insert(std::string const & name, ValueTypeBase const & value);
	void insert(ValueDictionary const & dictionary);
	void remove(std::string const & name);
	bool exists(std::string const & name) const;
	ValueTypeBase *getCopy(std::string const & name) const; // caller is responsible for freeing returned object
	void clear();
	bool empty() const;
	DictionaryValueMap const &get() const;

private:

	DictionaryValueMap m_valueMap;
};

// ----------------------------------------------------------------------

inline bool ValueDictionary::empty() const
{
	return m_valueMap.empty();
}

// ----------------------------------------------------------------------

inline DictionaryValueMap const & ValueDictionary::get() const
{
	return m_valueMap;
}

// ======================================================================

#endif
