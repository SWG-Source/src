// ======================================================================
//
// PooledString.h
//
// Copyright 2003 Sony Online Entertainment
//
// ======================================================================

#ifndef _PooledString_H_
#define _PooledString_H_

// ======================================================================

#include "Archive/Archive.h"

// ======================================================================

class PooledString
{
public:
	static void install();
	static void remove();

	PooledString();
	PooledString(PooledString const &s);
	PooledString(std::string const &s);
	PooledString(char const *s);
	PooledString &operator=(PooledString const &s);
	PooledString &operator=(std::string const &s);
	PooledString &operator=(char const *s);
	operator std::string const &() const;
	bool operator==(PooledString const &s) const;
	bool operator!=(PooledString const &s) const;
	bool operator<(PooledString const &s) const;
	char const *c_str() const;

private:
	std::string const *m_string;
};

// ======================================================================

namespace Archive
{
	void get(ReadIterator &source, PooledString &target);
	void put(ByteStream &target, PooledString const &source);
}

// ======================================================================

#endif // _PooledString_H_

