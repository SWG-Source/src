// ============================================================================
//
// MatchMakingCharacterProfileId.h
// Copyright Sony Online Entertainment
//
// ============================================================================

#ifndef INCLUDED_MatchMakingCharacterProfileId_H
#define INCLUDED_MatchMakingCharacterProfileId_H

#include "Archive/AutoByteStream.h"
#include "sharedGame/MatchMakingId.h"

//-----------------------------------------------------------------------------
class MatchMakingCharacterProfileId
{
public:

	MatchMakingCharacterProfileId();

	void                 setMatchMakingId(MatchMakingId const &matchMakindId);
	MatchMakingId const &getMatchMakingId() const;

	void                 unpackFromString(std::string &value);
	void                 packToString(std::string &value);

	bool                           operator ==(MatchMakingCharacterProfileId const &rhs) const;
	bool                           operator !=(MatchMakingCharacterProfileId const &rhs) const;
	MatchMakingCharacterProfileId &operator  =(MatchMakingCharacterProfileId const &rhs);

private:

	MatchMakingId m_matchMakingId;
};

//-----------------------------------------------------------------------------
namespace Archive
{
	void get(ReadIterator &source, MatchMakingCharacterProfileId &target);
	void put(ByteStream &target, const MatchMakingCharacterProfileId &source);
}

// ============================================================================

#endif // INCLUDED_MatchMakingCharacterProfileId_H
