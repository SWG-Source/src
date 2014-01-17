// ======================================================================
//
// GameServerData.h
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_GameServerData_H
#define INCLUDED_GameServerData_H

// ======================================================================

class GameServerConnection;

// ======================================================================

class GameServerData
{
public:
	explicit GameServerData(GameServerConnection *connection);
	GameServerData(const GameServerData &rhs);
	GameServerData();
	GameServerData &operator=(const GameServerData &rhs);

  public:
	enum ServerStatus {SS_unready, SS_readyForObjects, SS_loadedUniverseObjects, SS_running};
	
  public:
	GameServerConnection *       getConnection      ();
	int                          getObjectCount     () const;
	int                          getInterestObjectCount     () const;
	int                          getInterestCreatureObjectCount() const;
	ServerStatus                 getServerStatus    () const;
	uint32                       getProcessId       () const;
	void                         debugOutputData    () const;

	void                         adjustObjectCount  (int adjustment);
	void                         adjustInterestObjectCount  (int adjustment);
	void                         adjustInterestCreatureObjectCount  (int adjustment);
	void                         universeLoaded     ();
	void                         ready              ();
	void                         preloadComplete    ();

	bool                         isRunning          () const;

private:
	GameServerConnection *       m_connection;
	int                          m_objectCount;
	int                          m_interestObjectCount;
	int                          m_interestCreatureObjectCount;
	ServerStatus                 m_serverStatus;
};

// ======================================================================

inline GameServerConnection *GameServerData::getConnection()
{
	return m_connection;
}

// ----------------------------------------------------------------------

inline int GameServerData::getObjectCount() const
{
	return m_objectCount;
}

// ----------------------------------------------------------------------

inline void GameServerData::adjustObjectCount(int adjustment)
{
	m_objectCount+=adjustment;
}

// ----------------------------------------------------------------------

inline void GameServerData::adjustInterestObjectCount(int adjustment)
{
	m_interestObjectCount+=adjustment;
}

// ----------------------------------------------------------------------

inline GameServerData::ServerStatus GameServerData::getServerStatus() const
{
	return m_serverStatus;
}

// ----------------------------------------------------------------------

inline int GameServerData::getInterestObjectCount() const
{
	return m_interestObjectCount;
}

// ----------------------------------------------------------------------

inline bool GameServerData::isRunning() const
{
	return m_serverStatus == SS_running;
}

// ----------------------------------------------------------------------

inline int GameServerData::getInterestCreatureObjectCount() const
{
	return m_interestCreatureObjectCount;
}

// ----------------------------------------------------------------------

inline void GameServerData::adjustInterestCreatureObjectCount  (int adjustment)
{
	m_interestCreatureObjectCount+=adjustment;
}

// ======================================================================

#endif
