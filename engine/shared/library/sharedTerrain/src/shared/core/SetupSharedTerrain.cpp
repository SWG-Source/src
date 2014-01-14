//===================================================================
//
// SetupSharedTerrain.cpp
// asommers 9-10-2000
//
// copyright 2000, verant interactive
//
//===================================================================

#include "sharedTerrain/FirstSharedTerrain.h"
#include "sharedTerrain/SetupSharedTerrain.h"

#include "sharedDebug/InstallTimer.h"
#include "sharedFoundation/ExitChain.h"
#include "sharedFractal/MultiFractal.h"
#include "sharedTerrain/ConfigSharedTerrain.h"
#include "sharedTerrain/ServerProceduralTerrainAppearance.h"
#include "sharedTerrain/ServerProceduralTerrainAppearanceTemplate.h"
#include "sharedTerrain/ServerSpaceTerrainAppearanceTemplate.h"
#include "sharedTerrain/TerrainObject.h"
#include "sharedTerrain/WaterTypeManager.h"

//===================================================================

namespace
{
	bool ms_installed;
}

//===================================================================

SetupSharedTerrain::Data::Data () :
	m_allowInactiveLayerItems (false)
{
}

//===================================================================

void SetupSharedTerrain::install (const SetupSharedTerrain::Data& /*data*/)
{
	InstallTimer const installTimer("SetupSharedTerrain::install");

	DEBUG_FATAL (ms_installed, ("SetupSharedTerrain::install already installed"));
	ms_installed = true;

	DEBUG_REPORT_LOG_PRINT (ConfigSharedTerrain::getDebugReportInstall (), ("SetupSharedTerrain::install\n"));

	ConfigSharedTerrain::install ();

	TerrainObject::install ();
	ProceduralTerrainAppearance::install ();
	ServerProceduralTerrainAppearanceTemplate::install ();
	ServerSpaceTerrainAppearanceTemplate::install();
	WaterTypeManager::install();

	ExitChain::add (SetupSharedTerrain::remove, "SetupSharedTerrain");
}

//-------------------------------------------------------------------

void SetupSharedTerrain::remove ()
{
	DEBUG_FATAL (!ms_installed, ("SetupSharedTerrain::remove not installed"));
	ms_installed = false;
/*
#ifdef _DEBUG
	MultiFractal::debugDump ();
#endif
*/
}

//-------------------------------------------------------------------

bool SetupSharedTerrain::isInstalled ()
{
	return ms_installed;
}

//-------------------------------------------------------------------

void SetupSharedTerrain::setupGameData (Data& data)
{
	data.m_allowInactiveLayerItems = false;
}

//-------------------------------------------------------------------

void SetupSharedTerrain::setupToolData (Data& data)
{
	data.m_allowInactiveLayerItems = true;
}

//===================================================================
