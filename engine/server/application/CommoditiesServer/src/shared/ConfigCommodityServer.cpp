// ======================================================================
//
// ConfigCommodityServer.cpp
// Copyright 2000-01, Sony Online Entertainment Inc., all rights reserved.
// Author: Doug Mellencamp
//
// ======================================================================

//-----------------------------------------------------------------------

#include "FirstCommodityServer.h"
#include "sharedFoundation/ConfigFile.h"
#include "sharedNetwork/SetupSharedNetwork.h"
#include "ConfigCommodityServer.h"

//-----------------------------------------------------------------------

ConfigCommodityServer::Data *	ConfigCommodityServer::data = 0;

#define KEY_INT(a,b)    (data->a = ConfigFile::getKeyInt("CommodityServer", #a, b))
#define KEY_BOOL(a,b)   (data->a = ConfigFile::getKeyBool("CommodityServer", #a, b))
#define KEY_FLOAT(a,b)   (data->a = ConfigFile::getKeyFloat("CommodityServer", #a, b))
#define KEY_STRING(a,b) (data->a = ConfigFile::getKeyString("CommodityServer", #a, b))

//-----------------------------------------------------------------------

void ConfigCommodityServer::install(void)
{
	SetupSharedNetwork::SetupData  networkSetupData;
	SetupSharedNetwork::getDefaultServerSetupData(networkSetupData);

	data = new ConfigCommodityServer::Data;

	KEY_INT     (cmServerServiceBindPort, 4069);
	KEY_STRING  (cmServerServiceBindInterface, "localhost");
	KEY_INT     (databaseServerPort, 44457);	//TODO: confirm port
	KEY_STRING  (databaseServerAddress, "localhost");
	KEY_STRING  (dsn,"swodb");
	KEY_STRING  (databaseUID,"dmellencamp");
	KEY_STRING  (databasePWD, "compts6m");
	KEY_STRING  (databaseSchema, "dmellencamp");
	KEY_INT     (secondsBetweenDBReconnect, 45);
	KEY_STRING  (databaseProtocol, "OCI");
	KEY_BOOL    (enableQueryProfile,false);
	KEY_BOOL    (verboseQueryMode,false);
	KEY_BOOL    (developmentMode, true);
	KEY_INT     (databaseThreads, 1);
	KEY_INT     (databaseCharacterNameSizeLimit, 64);
	KEY_INT     (databaseItemNameSizeLimit, 256);
	KEY_INT     (databaseUserDescriptionSizeLimit, 1024);
	KEY_INT     (databaseOOBDataSizeLimit, 4000);
	KEY_BOOL    (snapshotDBWrite, true);
	KEY_BOOL    (showAllDebugInfo, true);
	KEY_INT     (centralServerPort, 44456);
	KEY_STRING  (centralServerAddress, "localhost");
	KEY_INT     (sleepTimePerFrameMs, 1);
	KEY_INT     (minutesActiveToUnaccessed, 10080); // 7 days
	KEY_INT     (minutesEmptyToEndangered, 10080);
	KEY_INT     (minutesUnaccessedToEndangered, 10080);
	KEY_INT     (minutesEndangeredToRemoved, 10080);
	KEY_INT     (minutesVendorAuctionTimer, 43200); // 30 days
	KEY_INT     (minutesVendorItemTimer, 43200);
	KEY_INT     (maxAuctionsPerPage, 100);
	KEY_INT     (minutesBazaarAuctionTimer, 10080);
	KEY_INT     (minutesBazaarItemTimer, 10080);
	KEY_INT     (maxAuctionsPerPlayer, 25);
}

//-----------------------------------------------------------------------

void ConfigCommodityServer::remove(void)
{
	delete data;
	data = 0;
}
