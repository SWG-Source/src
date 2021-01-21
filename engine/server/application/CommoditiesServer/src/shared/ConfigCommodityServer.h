// ======================================================================
//
// ConfigCommodityServer.h
// Copyright 2004, Sony Online Entertainment Inc., all rights reserved.
// Author: Doug Mellencamp
//
// ======================================================================

#ifndef	_ConfigCommodityServer_H
#define	_ConfigCommodityServer_H

//-----------------------------------------------------------------------

class ConfigCommodityServer
{
  public:
    struct Data
    {
	int           cmServerServiceBindPort;
	const char *  cmServerServiceBindInterface;
	int           databaseServerPort;
	const char *  databaseServerAddress;
	const char *  dsn;
	const char *  databaseUID;
	const char *  databasePWD;
	const char *  databaseSchema;
	int           secondsBetweenDBReconnect;
	const char *  databaseProtocol;
	bool          enableQueryProfile;
	bool          verboseQueryMode;
	bool          developmentMode;
	int           databaseThreads;                    // db task q threads
	int           databaseCharacterNameSizeLimit;     // max character name db column size
	int           databaseItemNameSizeLimit;          // max item name db column size
	int           databaseUserDescriptionSizeLimit;   // max user desc db column size
	int           databaseOOBDataSizeLimit;           // max out of band data db column size
	bool          snapshotDBWrite;                    // save db writes delayed or immediate
	bool          showAllDebugInfo;                   // show detailed debugging logging info
	int           centralServerPort;
	const char *  centralServerAddress;
	int           sleepTimePerFrameMs;
	int           minutesActiveToUnaccessed;
	int           minutesEmptyToEndangered;
	int           minutesUnaccessedToEndangered;
	int           minutesEndangeredToRemoved;
	int           minutesVendorAuctionTimer;        // max time an item is listed on a vendor
	int           minutesVendorItemTimer;           // max time a vendor item is held in stockroom after it expires for pickup by the seller (auction time + item time)
	int           maxAuctionsPerPage;
	int           minutesBazaarAuctionTimer;        // max time an item is listed on bazaar
	int           minutesBazaarItemTimer;           // max time a bazaar item is held by system after it expires for pickup by the seller (auction time + item time)
	int           maxAuctionsPerPlayer;             // max number of auctions a player can have active at any given time
    };

	static uint16        getCMServerServiceBindPort             ();
	static const char *  getCMServerServiceBindInterface        ();
	static uint16        getDatabaseServerPort                  ();
	static const char *  getDatabaseServerAddress               ();
	static const char *  getDSN                                 ();
	static const char *  getDatabaseUID                         ();
	static const char *  getDatabasePWD                         ();
	static const char *  getDatabaseSchema                      ();
	static uint16        getSecondsBetweenDBReconnect           ();
	static const char *  getDatabaseProtocol                    ();
	static bool          getEnableQueryProfile                  ();
	static bool          getVerboseQueryMode                    ();
	static bool          getDevelopmentMode                     ();
	static int           getDatabaseThreads                     ();
	static unsigned int  getDatabaseCharacterNameSizeLimit      ();
	static unsigned int  getDatabaseItemNameSizeLimit           ();
	static unsigned int  getDatabaseUserDescriptionSizeLimit    ();
	static unsigned int  getDatabaseOOBDataSizeLimit            ();
	static bool          getSnapshotDBWrite                     ();
	static bool          getShowAllDebugInfo                    ();
	static uint16        getCentralServerPort                   ();
	static const char *  getCentralServerAddress                ();
	static int           getSleepTimePerFrameMs                 ();
	static int           getMinutesActiveToUnaccessed           ();
	static int           getMinutesEmptyToEndangered            ();
	static int           getMinutesUnaccessedToEndangered       ();
	static int           getMinutesEndangeredToRemoved          ();
	static int           getMinutesVendorAuctionTimer           ();
	static int           getMinutesVendorItemTimer              ();
	static int           getMaxAuctionsPerPage                  ();
	static int           getMinutesBazaarAuctionTimer           ();
	static int           getMinutesBazaarItemTimer              ();
	static int           getMaxAuctionsPerPlayer                ();

	static void          install                                ();
	static void          remove                                 ();

  private:
    static Data *	data;
};

//-----------------------------------------------------------------------

inline unsigned short ConfigCommodityServer::getCMServerServiceBindPort()
{
	return data->cmServerServiceBindPort;
}

//-----------------------------------------------------------------------

inline unsigned short ConfigCommodityServer::getDatabaseServerPort()
{
	return data->databaseServerPort;
}

//-----------------------------------------------------------------------

inline const char * ConfigCommodityServer::getCMServerServiceBindInterface()
{
	return data->cmServerServiceBindInterface;
}

//-----------------------------------------------------------------------

inline const char * ConfigCommodityServer::getDatabaseServerAddress()
{
	return data->databaseServerAddress;
}

//-----------------------------------------------------------------------

inline const char * ConfigCommodityServer::getDSN()
{
	return data->dsn;
}

//-----------------------------------------------------------------------

inline const char * ConfigCommodityServer::getDatabaseUID()
{
	return data->databaseUID;
}

//-----------------------------------------------------------------------

inline const char * ConfigCommodityServer::getDatabasePWD()
{
	return data->databasePWD;
}

//-----------------------------------------------------------------------

inline const char * ConfigCommodityServer::getDatabaseSchema()
{
	return data->databaseSchema;
}

//-----------------------------------------------------------------------

inline unsigned short ConfigCommodityServer::getSecondsBetweenDBReconnect()
{
	return data->secondsBetweenDBReconnect;
}

//-----------------------------------------------------------------------

inline const char * ConfigCommodityServer::getDatabaseProtocol()
{
	return (data->databaseProtocol);
}

//-----------------------------------------------------------------------

inline bool ConfigCommodityServer::getEnableQueryProfile()
{
	return (data->enableQueryProfile);
}

// ----------------------------------------------------------------------

inline bool ConfigCommodityServer::getVerboseQueryMode()
{
	return (data->verboseQueryMode);
}

// ----------------------------------------------------------------------

inline bool ConfigCommodityServer::getDevelopmentMode()
{
	return (data->developmentMode);
}

// ----------------------------------------------------------------------

inline int ConfigCommodityServer::getDatabaseThreads()
{
	return (data->databaseThreads);
}

// ----------------------------------------------------------------------

inline unsigned int ConfigCommodityServer::getDatabaseCharacterNameSizeLimit()
{
	return (data->databaseCharacterNameSizeLimit);
}

// ----------------------------------------------------------------------

inline unsigned int ConfigCommodityServer::getDatabaseItemNameSizeLimit()
{
	return (data->databaseItemNameSizeLimit);
}

// ----------------------------------------------------------------------

inline unsigned int ConfigCommodityServer::getDatabaseUserDescriptionSizeLimit()
{
	return (data->databaseUserDescriptionSizeLimit);
}

// ----------------------------------------------------------------------

inline unsigned int ConfigCommodityServer::getDatabaseOOBDataSizeLimit()
{
	return (data->databaseOOBDataSizeLimit);
}

// ----------------------------------------------------------------------

inline bool ConfigCommodityServer::getSnapshotDBWrite()
{
	return (data->snapshotDBWrite);
}

// ----------------------------------------------------------------------

inline bool ConfigCommodityServer::getShowAllDebugInfo()
{
	return (data->showAllDebugInfo);
}

//-----------------------------------------------------------------------

inline unsigned short ConfigCommodityServer::getCentralServerPort()
{
	return data->centralServerPort;
}

//-----------------------------------------------------------------------

inline const char * ConfigCommodityServer::getCentralServerAddress()
{
	return data->centralServerAddress;
}

//-----------------------------------------------------------------------

inline int ConfigCommodityServer::getSleepTimePerFrameMs()
{
	return data->sleepTimePerFrameMs;
}

//-----------------------------------------------------------------------

inline int ConfigCommodityServer::getMinutesActiveToUnaccessed()
{
	return data->minutesActiveToUnaccessed;
}

//-----------------------------------------------------------------------

inline int ConfigCommodityServer::getMinutesEmptyToEndangered()
{
	return data->minutesEmptyToEndangered;
}

//-----------------------------------------------------------------------

inline int ConfigCommodityServer::getMinutesUnaccessedToEndangered()
{
	return data->minutesUnaccessedToEndangered;
}

//-----------------------------------------------------------------------

inline int ConfigCommodityServer::getMinutesEndangeredToRemoved()
{
	return data->minutesEndangeredToRemoved;
}

//-----------------------------------------------------------------------

inline int ConfigCommodityServer::getMinutesVendorAuctionTimer()
{
	return data->minutesVendorAuctionTimer;
}

//-----------------------------------------------------------------------

inline int ConfigCommodityServer::getMinutesVendorItemTimer()
{
	return data->minutesVendorItemTimer;
}

//-----------------------------------------------------------------------

inline int ConfigCommodityServer::getMaxAuctionsPerPage()
{
	return data->maxAuctionsPerPage;
}

//-----------------------------------------------------------------------

inline int ConfigCommodityServer::getMinutesBazaarAuctionTimer() {
    return data->minutesBazaarAuctionTimer;
}

//-----------------------------------------------------------------------

inline int ConfigCommodityServer::getMinutesBazaarItemTimer() {
    return data->minutesBazaarItemTimer;
}

//-----------------------------------------------------------------------

inline int ConfigCommodityServer::getMaxAuctionsPerPlayer() {
    return data->maxAuctionsPerPlayer;
}

#endif	// _ConfigCommodityServer_H
