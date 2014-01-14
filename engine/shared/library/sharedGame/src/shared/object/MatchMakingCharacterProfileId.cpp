// ============================================================================
//
// MatchMakingCharacterProfileId.cpp
// Copyright Sony Online Entertainment
//
// ============================================================================

#include "sharedGame/FirstSharedGame.h"
#include "sharedGame/MatchMakingCharacterProfileId.h"

// ============================================================================
//
// Archive
//
// ============================================================================

//-----------------------------------------------------------------------------
void Archive::get(ReadIterator &source, MatchMakingCharacterProfileId &target)
{
	// Match making id

	MatchMakingId::IntVector ints;
	get(source, ints);
	MatchMakingId matchMakingId;
	matchMakingId.setBits(ints);
	target.setMatchMakingId(matchMakingId);
}

//-----------------------------------------------------------------------------
void Archive::put(ByteStream &target, const MatchMakingCharacterProfileId &source)
{
	// Match making id

	put(target, source.getMatchMakingId().getInts());
}

// ============================================================================
//
// MatchMakingCharacterProfileId
//
// ============================================================================

//-----------------------------------------------------------------------------
MatchMakingCharacterProfileId::MatchMakingCharacterProfileId()
 : m_matchMakingId()
{
}

//-----------------------------------------------------------------------------
void MatchMakingCharacterProfileId::setMatchMakingId(MatchMakingId const &matchMakindId)
{
	m_matchMakingId = matchMakindId;
}

//-----------------------------------------------------------------------------
MatchMakingId const &MatchMakingCharacterProfileId::getMatchMakingId() const
{
	return m_matchMakingId;
}

//-----------------------------------------------------------------------------
void MatchMakingCharacterProfileId::unpackFromString(std::string &value)
{
	UNREF(value);
}

//-----------------------------------------------------------------------------
void MatchMakingCharacterProfileId::packToString(std::string &value)
{
	UNREF(value);
}

//-----------------------------------------------------------------------------
bool MatchMakingCharacterProfileId::operator ==(MatchMakingCharacterProfileId const &rhs) const
{
	return (m_matchMakingId == rhs.m_matchMakingId);
}

//-----------------------------------------------------------------------------
bool MatchMakingCharacterProfileId::operator !=(MatchMakingCharacterProfileId const &rhs) const
{
	return !(*this == rhs);
}

//-----------------------------------------------------------------------------
MatchMakingCharacterProfileId &MatchMakingCharacterProfileId::operator =(MatchMakingCharacterProfileId const &rhs)
{
	if (this != &rhs)
	{
		m_matchMakingId = rhs.m_matchMakingId;
	}

	return *this;
}

// ============================================================================
