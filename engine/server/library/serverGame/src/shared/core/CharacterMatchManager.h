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
	static void getMatchStatistics(unsigned long &numberOfCharacterMatchRequests, unsigned long &numberOfCharacterMatchResults, unsigned long &timeSpentOnCharacterMatchRequestsMs);
	static void clearMatchStatistics();

private:
	
	static unsigned long ms_numberOfCharacterMatchRequests;
	static unsigned long ms_numberOfCharacterMatchResults;
	static unsigned long ms_timeSpentOnCharacterMatchRequestsMs;

	// Disable

	CharacterMatchManager();
	~CharacterMatchManager();
	CharacterMatchManager(CharacterMatchManager const &);
	CharacterMatchManager &operator =(CharacterMatchManager const &);
};

//-----------------------------------------------------------------------

inline void CharacterMatchManager::getMatchStatistics(unsigned long &numberOfCharacterMatchRequests, unsigned long &numberOfCharacterMatchResults, unsigned long &timeSpentOnCharacterMatchRequestsMs)
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
