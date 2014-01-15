// ======================================================================
//
// PortalTriggerVolume.cpp
//
// Copyright 2000-04 Sony Online Entertainment
//
// ======================================================================

#include "serverGame/FirstServerGame.h"
#include "PortalTriggerVolume.h"

#include "serverGame/ObserveTracker.h"
#include "serverGame/ServerObject.h"

// ======================================================================

namespace PortalTriggerVolumeNamespace
{
	const char* ms_name = "portalTriggerVolume";
}

using namespace PortalTriggerVolumeNamespace;

// ======================================================================

PortalTriggerVolume::PortalTriggerVolume(ServerObject &owner, real radius) :
	TriggerVolume(owner, radius, ms_name, true)
{
}

// ----------------------------------------------------------------------

PortalTriggerVolume::~PortalTriggerVolume()
{
}

// ----------------------------------------------------------------------

const char* PortalTriggerVolume::getName()
{
	return ms_name;
}

// ----------------------------------------------------------------------

void PortalTriggerVolume::virtualOnEnter(ServerObject &object)
{
	Client *client = object.getClient();
	if (client)
		ObserveTracker::onClientEnteredPortalTriggerVolume(*client, *this);
}

// ----------------------------------------------------------------------

void PortalTriggerVolume::virtualOnExit(ServerObject &object)
{
	Client *client = object.getClient();
	if (client)
		ObserveTracker::onClientExitedPortalTriggerVolume(*client, *this);
}

// ----------------------------------------------------------------------

bool PortalTriggerVolume::isPortalTriggerVolume() const
{
	return true;
}

// ======================================================================

