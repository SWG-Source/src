// GameGameServerMessages.h
// copyright 2001 Verant Interactive

#ifndef	_INCLUDED_GameGameServerMessages_H
#define	_INCLUDED_GameGameServerMessages_H

#include "sharedNetworkMessages/GameNetworkMessage.h"

//-----------------------------------------------------------------------

class GameGameServerConnect : public GameNetworkMessage
{
public:
	GameGameServerConnect  (uint32 processId, bool isDbProcess, int spawnCookie, int preloadNumber);
	GameGameServerConnect  (Archive::ReadIterator & source);
	~GameGameServerConnect ();

	bool                 getIsDbProcess () const;
	const uint32		 getProcessId   () const;
	int                  getSpawnCookie () const;
	int                  getPreloadNumber() const;

private:
	Archive::AutoVariable<bool>   m_isDbProcess;
	Archive::AutoVariable<uint32> m_processId;
	Archive::AutoVariable<int>    m_spawnCookie;
	Archive::AutoVariable<int>    m_preloadNumber;
	GameGameServerConnect();
	GameGameServerConnect(const GameGameServerConnect&);
	GameGameServerConnect& operator= (const GameGameServerConnect&);
};

//-----------------------------------------------------------------------

inline bool GameGameServerConnect::getIsDbProcess() const
{
	return m_isDbProcess.get();
}

//-----------------------------------------------------------------------
inline const uint32 GameGameServerConnect::getProcessId() const
{
	return m_processId.get(); //lint !e1037 // const op () const is hardly ambiguous
}	// GameGameServerConnect::getProcessId

//-----------------------------------------------------------------------

inline int GameGameServerConnect::getSpawnCookie() const
{
	return m_spawnCookie.get();
}

//-----------------------------------------------------------------------

inline int GameGameServerConnect::getPreloadNumber() const
{
	return m_preloadNumber.get();
}

//-----------------------------------------------------------------------

#endif	// _INCLUDED_GameGameServerMessages_H
