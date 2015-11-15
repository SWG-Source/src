// ======================================================================
//
// SetupSharedNetwork.cpp
// Copyright 2003 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#include "sharedNetwork/FirstSharedNetwork.h"
#include "sharedNetwork/SetupSharedNetwork.h"

#include "sharedDebug/InstallTimer.h"
#include "sharedFoundation/ExitChain.h"
#include "sharedNetwork/ConfigSharedNetwork.h"

// ======================================================================

namespace SetupSharedNetworkNamespace
{
	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	void remove();

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	bool  s_installed;
}

using namespace SetupSharedNetworkNamespace;

// ======================================================================

void SetupSharedNetworkNamespace::remove()
{
	DEBUG_FATAL(!s_installed, ("SetupSharedNetworkNamespace not installed."));
	s_installed = false;
}

// ======================================================================

void SetupSharedNetwork::getDefaultClientSetupData(SetupData &data)
{
	//-- It is extremely important to have this value setup correctly for the client and server.
	//   The client should not be able to muck with this since it influences things like
	//   movement validation.
	data.m_clockSyncDelay = 45000;
}

// ----------------------------------------------------------------------

void SetupSharedNetwork::getDefaultServerSetupData(SetupData &data)
{
	//-- It is extremely important to have this value setup correctly for the client and server.
	//   The client should not be able to muck with this since it influences things like
	//   movement validation.
	data.m_clockSyncDelay = 0;
}

// ----------------------------------------------------------------------

void SetupSharedNetwork::install(SetupData &setupData)
{
	InstallTimer const installTimer("SetupSharedNetwork::install");

	DEBUG_FATAL(s_installed, ("SetupSharedNetwork already installed."));

	ConfigSharedNetwork::install(setupData.m_clockSyncDelay);

	s_installed = true;
	ExitChain::add(remove, "SetupSharedNetwork");
}

// ======================================================================
