// ======================================================================
//
// UniqueNameList.cpp
// copyright 2001 Sony Online Entertainment
// 
// ======================================================================

#include "sharedUtility/FirstSharedUtility.h"
#include "sharedUtility/UniqueNameList.h"

#include "sharedFoundation/CrcLowerString.h"

#include <algorithm>
#include <memory>
#include <utility>
#include <vector>

// ======================================================================
// lint supression
// ======================================================================

//lint -e18 // error: symbol redefined // not sure why I'm getting this

// ======================================================================

struct UniqueNameList::NameInfo
{
public:

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	struct LessNameComparator
	{
		bool operator ()(const std::shared_ptr<NameInfo> &lhs, const std::shared_ptr<NameInfo> &rhs) const;

		bool operator ()(const std::shared_ptr<CrcLowerString> &lhs, const std::shared_ptr<NameInfo> &rhs) const;
		bool operator ()(const std::shared_ptr<NameInfo> &lhs, const std::shared_ptr<CrcLowerString> &rhs) const;

		bool operator ()(const std::shared_ptr<NameInfo> &lhs, const CrcLowerString &rhs) const;
		bool operator ()(const CrcLowerString &lhs, const std::shared_ptr<NameInfo> &rhs) const;
	};

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

public:

	NameInfo(const std::shared_ptr<CrcLowerString> &name, int index);

public:

	std::shared_ptr<CrcLowerString>  m_name;
	int                                m_index;

private:
	// disabled
	NameInfo();
};

// ======================================================================
// struct UniqueNameList::NameInfo::LessNameComparator
// ======================================================================

//lint -esym(1714, LessNameComparator::operator*) // not referenced // wrong, in STL functions

inline bool UniqueNameList::NameInfo::LessNameComparator::operator ()(const std::shared_ptr<NameInfo> &lhs, const std::shared_ptr<NameInfo> &rhs) const
{
	return *(lhs->m_name.get()) < *(rhs->m_name.get());
}

// ----------------------------------------------------------------------

inline bool UniqueNameList::NameInfo::LessNameComparator::operator ()(const std::shared_ptr<CrcLowerString> &lhs, const std::shared_ptr<NameInfo> &rhs) const
{
	return *(lhs.get()) < *(rhs->m_name.get());
}

// ----------------------------------------------------------------------

inline bool UniqueNameList::NameInfo::LessNameComparator::operator ()(const std::shared_ptr<NameInfo> &lhs, const std::shared_ptr<CrcLowerString> &rhs) const
{
	return *(lhs->m_name.get()) < *(rhs.get());
}

// ----------------------------------------------------------------------

inline bool UniqueNameList::NameInfo::LessNameComparator::operator ()(const std::shared_ptr<NameInfo> &lhs, const CrcLowerString &rhs) const
{
	return *(lhs->m_name.get()) < rhs;
}

// ----------------------------------------------------------------------

inline bool UniqueNameList::NameInfo::LessNameComparator::operator ()(const CrcLowerString &lhs, const std::shared_ptr<NameInfo> &rhs) const
{
	return lhs < *(rhs->m_name.get());
}

// ======================================================================
// class UniqueNameList::NameInfo
// ======================================================================

UniqueNameList::NameInfo::NameInfo(const std::shared_ptr<CrcLowerString> &name, int index)
:	m_name(name),
	m_index(index)
{
}

// ======================================================================
// class UniqueNameList
// ======================================================================

UniqueNameList::UniqueNameList()
:	m_nameInfoByName(new NameInfoVector()),
	m_nameInfoByIndex(new NameInfoVector())
{
}

// ----------------------------------------------------------------------

UniqueNameList::~UniqueNameList()
{
	delete m_nameInfoByIndex;
	delete m_nameInfoByName;
}

// ----------------------------------------------------------------------
/**
 * Retrieve the unique integer assigned to the given name, adding the
 * name and assigning a new ID if the name doesn't already exist.
 *
 * @param name  the name to look up, added to the unique name list if not present.
 *
 * @return      the integer ID associated with the given name.
 */

int UniqueNameList::submitName(const SharedCrcLowerString &name)
{
	//-- see if this is a new name
	std::pair<NameInfoVector::iterator, NameInfoVector::iterator> result = std::equal_range(m_nameInfoByName->begin(), m_nameInfoByName->end(), name, NameInfo::LessNameComparator()); //lint !e64 // type mismatch on initialization // can't read message
	if (result.first == result.second)
	{
		// this is a new name

		//-- create it
		const int newIndex = static_cast<int>(m_nameInfoByName->size());
		
		std::shared_ptr<NameInfo>  newNameInfo = std::make_shared<NameInfo>(name, newIndex);

		//-- add to lists
		IGNORE_RETURN(m_nameInfoByName->insert(result.first, newNameInfo));
		m_nameInfoByIndex->push_back(newNameInfo);

		return newIndex;
	}
	else
	{
		//-- element already exists
		return (*result.first)->m_index;
	}
}

// ----------------------------------------------------------------------
/**
 * Find the unique integer assigned to the given name.
 *
 * This function will not add the name if it doesn't already exist.
 *
 * @param name   the name to look up.
 * @param index  if the name exists in the unique name list, the unique
 *               integer assigned to the name is returned in this parameter.
 *
 * @return       true if the name exists in the list, false otherwise.
 */

bool UniqueNameList::findName(const CrcLowerString &name, int &index) const
{
	NameInfoVector::const_iterator lowerBoundIt = std::lower_bound(m_nameInfoByName->begin(), m_nameInfoByName->end(), name, NameInfo::LessNameComparator());
	if ((lowerBoundIt == m_nameInfoByName->end()) || NameInfo::LessNameComparator()(name, *lowerBoundIt))
	{
		// not found
		index = -1;
		return false;
	}
	else
	{
		index = (*lowerBoundIt)->m_index;
		return true;
	}
}

// ----------------------------------------------------------------------

int UniqueNameList::getUniqueNameCount() const
{
	return static_cast<int>(m_nameInfoByIndex->size());
}

// ----------------------------------------------------------------------

const CrcLowerString &UniqueNameList::getName(int index) const
{
	VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE(0, index, static_cast<int>(m_nameInfoByIndex->size()));

	return *((*m_nameInfoByIndex)[static_cast<size_t>(index)]->m_name.get());
}

// ----------------------------------------------------------------------
/**
 * Reset the container to its initial state.
 *
 * All names are forgotten and new names start counting at zero.  The
 * container behaves as it did when it was first initialized.
 */ 

void UniqueNameList::reset()
{
	m_nameInfoByName->clear();
	m_nameInfoByIndex->clear();
}

// ======================================================================
