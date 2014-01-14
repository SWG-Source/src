// ============================================================================
//
// MatchMakingCharacterPreferenceId.h
// Copyright Sony Online Entertainment
//
// ============================================================================

#ifndef INCLUDED_MatchMakingCharacterPreferenceId_H
#define INCLUDED_MatchMakingCharacterPreferenceId_H

#include "Archive/AutoByteStream.h"
#include "sharedFoundation/BitArray.h"

//-----------------------------------------------------------------------------
class MatchMakingCharacterPreferenceId
{
public:

	MatchMakingCharacterPreferenceId();

	void                 setSubString(std::string const &subString);
	std::string const &  getSubString() const;

	BitArray const &     getSearchAttribute() const;
	BitArray &           getSearchAttribute();

	void                 packString(std::string &value) const;
	void                 unPackString(std::string const &value);

	bool                              operator ==(MatchMakingCharacterPreferenceId const &rhs) const;
	bool                              operator !=(MatchMakingCharacterPreferenceId const &rhs) const;
	MatchMakingCharacterPreferenceId &operator  =(MatchMakingCharacterPreferenceId const &rhs);

private:

	BitArray m_searchAttribute;
	std::string m_subString;
};

//-----------------------------------------------------------------------------

namespace Archive
{
	void get(ReadIterator &source, MatchMakingCharacterPreferenceId &target);
	void put(ByteStream &target, const MatchMakingCharacterPreferenceId &source);
}

//-----------------------------------------------------------------------------

inline void MatchMakingCharacterPreferenceId::setSubString(std::string const &subString)
{
	m_subString = subString;
}

//-----------------------------------------------------------------------------

inline std::string const & MatchMakingCharacterPreferenceId::getSubString() const
{
	return m_subString;
}

//-----------------------------------------------------------------------------

inline BitArray const & MatchMakingCharacterPreferenceId::getSearchAttribute() const
{
	return m_searchAttribute;
}

//-----------------------------------------------------------------------------

inline BitArray & MatchMakingCharacterPreferenceId::getSearchAttribute()
{
	return m_searchAttribute;
}

// ============================================================================

#endif // INCLUDED_MatchMakingCharacterPreferenceId_H
