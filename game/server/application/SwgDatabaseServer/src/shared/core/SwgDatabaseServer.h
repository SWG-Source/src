// ======================================================================
//
// SwgDatabaseServer.h
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_SwgDatabaseServer_H
#define INCLUDED_SwgDatabaseServer_H

#include "serverDatabase/DatabaseProcess.h"

// ======================================================================

class SwgDatabaseServer : public DatabaseProcess
{
  public:
	static void install();

	virtual void run();
	static void runStatic();

	virtual void frameTick();

	void setupSwgMetrics();
	void tickSwgMetrics();

  private:
	SwgDatabaseServer();
	virtual ~SwgDatabaseServer();

  public:
        int m_metricTableBufferBattlefieldMarkerObjectBufferCreated;
        int m_metricTableBufferBattlefieldMarkerObjectBufferActive;

        int m_metricTableBufferBuildingObjectBufferCreated;
        int m_metricTableBufferBuildingObjectBufferActive;

        int m_metricTableBufferCellObjectBufferCreated;
        int m_metricTableBufferCellObjectBufferActive;

        int m_metricTableBufferCityObjectBufferCreated;
        int m_metricTableBufferCityObjectBufferActive;

        int m_metricTableBufferCreatureObjectBufferCreated;
        int m_metricTableBufferCreatureObjectBufferActive;

        int m_metricTableBufferFactoryObjectBufferCreated;
        int m_metricTableBufferFactoryObjectBufferActive;

        int m_metricTableBufferGuildObjectBufferCreated;
        int m_metricTableBufferGuildObjectBufferActive;

        int m_metricTableBufferHarvesterInstallationObjectBufferCreated;
        int m_metricTableBufferHarvesterInstallationObjectBufferActive;

        int m_metricTableBufferInstallationObjectBufferCreated;
        int m_metricTableBufferInstallationObjectBufferActive;

        int m_metricTableBufferIntangibleObjectBufferCreated;
        int m_metricTableBufferIntangibleObjectBufferActive;

        int m_metricTableBufferManufactureInstallationObjectBufferCreated;
        int m_metricTableBufferManufactureInstallationObjectBufferActive;

        int m_metricTableBufferManufactureSchematicObjectBufferCreated;
        int m_metricTableBufferManufactureSchematicObjectBufferActive;

        int m_metricTableBufferMissionObjectBufferCreated;
        int m_metricTableBufferMissionObjectBufferActive;

        int m_metricTableBufferObjectTableBufferCreated;
        int m_metricTableBufferObjectTableBufferActive;

        int m_metricTableBufferPlanetObjectBufferCreated;
        int m_metricTableBufferPlanetObjectBufferActive;

        int m_metricTableBufferPlayerObjectBufferCreated;
        int m_metricTableBufferPlayerObjectBufferActive;

        int m_metricTableBufferResourceContainerObjectBufferCreated;
        int m_metricTableBufferResourceContainerObjectBufferActive;

        int m_metricTableBufferShipObjectBufferCreated;
        int m_metricTableBufferShipObjectBufferActive;

        int m_metricTableBufferStaticObjectBufferCreated;
        int m_metricTableBufferStaticObjectBufferActive;

        int m_metricTableBufferTangibleObjectBufferCreated;
        int m_metricTableBufferTangibleObjectBufferActive;

        int m_metricTableBufferUniverseObjectBufferCreated;
        int m_metricTableBufferUniverseObjectBufferActive;

        int m_metricTableBufferVehicleObjectBufferCreated;
        int m_metricTableBufferVehicleObjectBufferActive;

        int m_metricTableBufferWeaponObjectBufferCreated;
        int m_metricTableBufferWeaponObjectBufferActive;

	// Auction metrics
	int m_metricAuctionLocationsRowCreated;
	int m_metricAuctionLocationsRowActive;

	int m_metricMarketAuctionsRowCreated;
	int m_metricMarketAuctionsRowActive;

	int m_metricMarketAuctionBidsRowCreated;
	int m_metricMarketAuctionBidsRowActive;

	// Objvar
	int  m_metricObjvarNameRowsActive;
	int  m_metricGoldObjvarNameRowsActive;
};

// ======================================================================

#endif
