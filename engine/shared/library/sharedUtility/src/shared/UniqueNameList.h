// ======================================================================
//
// UniqueNameList.h
// copyright 2001 Sony Online Entertainment, Inc.
// All rights reserved.
// 
// ======================================================================

#ifndef INCLUDED_UniqueNameList_H
#define INCLUDED_UniqueNameList_H

// ======================================================================

class CrcLowerString;

namespace std
{
	template <typename T>
	class shared_ptr;
}

// ======================================================================
/**
 * Provides an interface for maintaining a unique set of names, assinging
 * a monotonically increasing index to each unique name.
 *
 * This class is useful when you want two fast-access mappings:
 * (1) unique name to a stable integer index, and (2) integer index
 * mapped back to unique name.  Names are lower case, case insensitive.
 *
 * This class makes use of shared_ptr to minimize the number of
 * memory allocations associated with using a CrcLowerString.  It also
 * makes use of sorted vectors for storage.
 */

class UniqueNameList
{
public:

	typedef std::shared_ptr<CrcLowerString>  SharedCrcLowerString;

public:

	UniqueNameList();
	~UniqueNameList();

	int                   submitName(const SharedCrcLowerString &name);
	bool                  findName(const CrcLowerString &name, int &index) const;

	int                   getUniqueNameCount() const;
	const CrcLowerString &getName(int index) const;

	void                  reset();

private:

	struct NameInfo;

	typedef std::vector<std::shared_ptr<NameInfo> >  NameInfoVector;

private:

	NameInfoVector *m_nameInfoByName;
	NameInfoVector *m_nameInfoByIndex;

private:
	// disabled
	UniqueNameList(const UniqueNameList&);
	UniqueNameList &operator =(const UniqueNameList&);
};

// ======================================================================

#endif
