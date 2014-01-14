// ============================================================================
//
// MatchMakingCharacterResult.h
// Copyright Sony Online Entertainment
//
// ============================================================================

#ifndef INCLUDED_MatchMakingCharacterResult_H
#define INCLUDED_MatchMakingCharacterResult_H

#include "sharedGame/LfgCharacterData.h"

//-----------------------------------------------------------------------------
struct MatchMakingCharacterResult
{
	MatchMakingCharacterResult();
	~MatchMakingCharacterResult();

	std::vector<LfgCharacterSearchResultData> m_matchingCharacterData;
	std::map<NetworkId, Unicode::String> m_matchingCharacterBiography;

	struct MatchMakingCharacterGroupMemberInfo
	{
		NetworkId groupMemberId;
		std::string groupMemberName;
		int groupMemberLevel;
		uint8 groupMemberProfession;
	};

	std::map<NetworkId, std::vector<MatchMakingCharacterResult::MatchMakingCharacterGroupMemberInfo> > m_matchingCharacterGroup;
	bool m_hasMoreMatches;
};

//-----------------------------------------------------------------------------

namespace Archive
{
	class ReadIterator;
	class ByteStream;

	void get(ReadIterator &source, MatchMakingCharacterResult &target);
	void put(ByteStream &target, const MatchMakingCharacterResult &source);

	void get(ReadIterator &source, MatchMakingCharacterResult::MatchMakingCharacterGroupMemberInfo &target);
	void put(ByteStream &target, const MatchMakingCharacterResult::MatchMakingCharacterGroupMemberInfo &source);
}

// ============================================================================

#endif // INCLUDED_MatchMakingCharacterResult_H
