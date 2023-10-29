// ============================================================================
// 
// CharacterMatchManager.h
// Copyright Sony Online Entertainment, Inc.
//
// ============================================================================

#ifndef INCLUDED_CharacterMatchManager_H
#define INCLUDED_CharacterMatchManager_H

class MatchMakingCharacterPreferenceId;
class NetworkId;

//-----------------------------------------------------------------------------
class CharacterMatchManager
{
public:

	static void requestMatch(NetworkId const &networkId, MatchMakingCharacterPreferenceId const &matchMakingCharacterPreferenceId);
	static void getMatchStatistics(uint32_t &numberOfCharacterMatchRequests, uint32_t &numberOfCharacterMatchResults, uint32_t &timeSpentOnCharacterMatchRequestsMs);
	static void clearMatchStatistics();

private:
	
	static uint32_t ms_numberOfCharacterMatchRequests;
	static uint32_t ms_numberOfCharacterMatchResults;
	static uint32_t ms_timeSpentOnCharacterMatchRequestsMs;

	// Disable

	CharacterMatchManager();
	~CharacterMatchManager();
	CharacterMatchManager(CharacterMatchManager const &);
	CharacterMatchManager &operator =(CharacterMatchManager const &);
};

//-----------------------------------------------------------------------

inline void CharacterMatchManager::getMatchStatistics(uint32_t &numberOfCharacterMatchRequests, uint32_t &numberOfCharacterMatchResults, uint32_t &timeSpentOnCharacterMatchRequestsMs)
{
	numberOfCharacterMatchRequests = ms_numberOfCharacterMatchRequests;
	numberOfCharacterMatchResults = ms_numberOfCharacterMatchResults;
	timeSpentOnCharacterMatchRequestsMs = ms_timeSpentOnCharacterMatchRequestsMs;
}

//-----------------------------------------------------------------------

inline void CharacterMatchManager::clearMatchStatistics()
{
	ms_numberOfCharacterMatchRequests = 0;
	ms_numberOfCharacterMatchResults = 0;
	ms_timeSpentOnCharacterMatchRequestsMs = 0;
}

// ============================================================================

#endif // INCLUDED_CharacterMatchManager_H
