// main.cpp
// Copyright 2000-02, Sony Online Entertainment Inc., all rights reserved.
// Author: Justin Randall

//-----------------------------------------------------------------------

#include "FirstCommodityServer.h"

#include "sharedFoundation/FirstSharedFoundation.h"
#include "sharedCompression/SetupSharedCompression.h"
#include "sharedDebug/SetupSharedDebug.h"
#include "sharedFile/TreeFile.h"
#include "sharedFile/SetupSharedFile.h"
#include "sharedFoundation/ConfigFile.h"
#include "sharedFoundation/ExitChain.h"
#include "sharedFoundation/Os.h"
#include "sharedFoundation/SetupSharedFoundation.h"
#include "sharedGame/CommoditiesAdvancedSearchAttribute.h"
#include "sharedGame/ConfigSharedGame.h"
#include "sharedNetwork/SetupSharedNetwork.h"
#include "sharedNetwork/NetworkHandler.h"
#include "sharedNetworkMessages/SetupSharedNetworkMessages.h"
#include "sharedThread/SetupSharedThread.h"
#include "sharedUtility/DataTableManager.h"
#include "CommodityServer.h"
#include "ConfigCommodityServer.h"
#include "LocalizationManager.h"
#include "UnicodeUtils.h"

//-----------------------------------------------------------------------

int main(int argc, char ** argv)
{
	SetupSharedThread::install();
	SetupSharedDebug::install(1024);

	//-- setup foundation
	SetupSharedFoundation::Data setupFoundationData(SetupSharedFoundation::Data::D_game);

	setupFoundationData.argc                      = argc;
	setupFoundationData.argv                      = argv;
	setupFoundationData.runInBackground           = true;
	SetupSharedFoundation::install (setupFoundationData);

	ConfigSharedGame::install();
	SetupSharedCompression::install();
	SetupSharedFile::install(false);
	SetupSharedNetworkMessages::install();

	SetupSharedNetwork::SetupData  networkSetupData;
	SetupSharedNetwork::getDefaultServerSetupData(networkSetupData);
	SetupSharedNetwork::install(networkSetupData);
	NetworkHandler::install();

	Os::setProgramName("CommodityServer");
	ConfigCommodityServer::install();

	const bool displayBadStringIds   = ConfigSharedGame::getDisplayBadStringIds ();
	const bool debugStringIds        = ConfigSharedGame::getDebugStringIds      ();
	Unicode::NarrowString defaultLocale(ConfigSharedGame::getDefaultLocale ());
	Unicode::UnicodeNarrowStringVector localeVector;
	localeVector.push_back(defaultLocale);

	LocalizationManager::install (new TreeFile::TreeFileFactory, localeVector, debugStringIds, nullptr, displayBadStringIds);
	ExitChain::add(LocalizationManager::remove, "LocalizationManager::remove");

	DataTableManager::install();
	CommoditiesAdvancedSearchAttribute::install();

	//-- run server
	SetupSharedFoundation::callbackWithExceptionHandling(CommodityServer::run);

	NetworkHandler::remove();
	SetupSharedFoundation::remove();
	SetupSharedThread::remove();

	return 0;
}

//-----------------------------------------------------------------------
