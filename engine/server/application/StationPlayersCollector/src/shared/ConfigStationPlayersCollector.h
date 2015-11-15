// ======================================================================
//
// ConfigStationPlayersCollector.h
// Copyright 2005, Sony Online Entertainment Inc., all rights reserved.
// Author: Doug Mellencamp
//
// ======================================================================

#ifndef	_ConfigStationPlayersCollector_H
#define	_ConfigStationPlayersCollector_H

//-----------------------------------------------------------------------

class ConfigStationPlayersCollector
{
  public:
    struct Data
    {
	int           centralServerServiceBindPort;
	const char *  centralServerServiceBindInterface;
	const char *  dsn;
	const char *  databaseUID;
	const char *  databasePWD;
	const char *  databaseSchema;
	int           secondsBetweenDBReconnect;
	const char *  databaseProtocol;
	bool          enableQueryProfile;
	bool          verboseQueryMode;
	int           databaseThreads;                    // db task q threads
	bool          logWorkerThreads;
	float         defaultDBQueueUpdateTimeLimit;
	int           disconnectSleepTime;
	bool          showAllDebugInfo;
	

    };

	static uint16        getCentralServerServiceBindPort           ();
	static const char *  getCentralServerServiceBindInterface      ();
	static const char *  getDSN                                 ();
	static const char *  getDatabaseUID                         ();
	static const char *  getDatabasePWD                         ();
	static const char *  getDatabaseSchema                      ();
	static uint16        getSecondsBetweenDBReconnect           ();
	static const char *  getDatabaseProtocol                    ();
	static const bool    getEnableQueryProfile                  ();
	static const bool    getVerboseQueryMode                    ();
	static int           getDatabaseThreads                     ();
	static const bool    getLogWorkerThreads                    ();
	static float         getDefaultDBQueueUpdateTimeLimit       ();
	static const int     getDisconnectSleepTime                 ();
	static const bool    getShowAllDebugInfo                    ();
	
	static void          install                                ();
	static void          remove                                 ();

  private:
    static Data *	data;
};

// ----------------------------------------------------------------------

#endif	// _ConfigStationPlayersCollector_H
