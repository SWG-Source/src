// ============================================================================
//
// MatchMakingCharacterResult.cpp
// Copyright Sony Online Entertainment
//
// ============================================================================

#include "sharedGame/FirstSharedGame.h"
#include "sharedGame/MatchMakingCharacterResult.h"

#include "sharedFoundation/NetworkIdArchive.h"
#include "unicodeArchive/UnicodeArchive.h"

// ============================================================================

MatchMakingCharacterResult::MatchMakingCharacterResult() :
	m_matchingCharacterData(),
	m_matchingCharacterBiography(),
	m_matchingCharacterGroup(),
	m_hasMoreMatches(false)
{
}

//-----------------------------------------------------------------------------

MatchMakingCharacterResult::~MatchMakingCharacterResult()
{
}

// ============================================================================
//
// Archive
//
// ============================================================================

//-----------------------------------------------------------------------------

void Archive::get(ReadIterator &source, MatchMakingCharacterResult::MatchMakingCharacterGroupMemberInfo &target)
{
	get(source, target.groupMemberId);
	get(source, target.groupMemberName);
	get(source, target.groupMemberLevel);
	get(source, target.groupMemberProfession);
}

//-----------------------------------------------------------------------------

void Archive::put(ByteStream &target, const MatchMakingCharacterResult::MatchMakingCharacterGroupMemberInfo &source)
{
	put(target, source.groupMemberId);
	put(target, source.groupMemberName);
	put(target, source.groupMemberLevel);
	put(target, source.groupMemberProfession);
}

//-----------------------------------------------------------------------------

void Archive::get(ReadIterator &source, MatchMakingCharacterResult &target)
{	
	get(source, target.m_matchingCharacterData);
	get(source, target.m_matchingCharacterBiography);
	get(source, target.m_matchingCharacterGroup);
	get(source, target.m_hasMoreMatches);
}

//-----------------------------------------------------------------------------

void Archive::put(ByteStream &target, const MatchMakingCharacterResult &source)
{	
	put(target, source.m_matchingCharacterData);
	put(target, source.m_matchingCharacterBiography);
	put(target, source.m_matchingCharacterGroup);
	put(target, source.m_hasMoreMatches);
}

// ============================================================================
