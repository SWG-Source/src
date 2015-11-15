// NetworkTriggerVolume.cpp
// Copyright 2000-01, Sony Online Entertainment Inc., all rights reserved. 

//-----------------------------------------------------------------------

#include "serverGame/FirstServerGame.h"
#include "NetworkTriggerVolume.h"

#include "serverGame/ObserveTracker.h"
#include "serverGame/ServerObject.h"

//-----------------------------------------------------------------------

NetworkTriggerVolume::NetworkTriggerVolume(ServerObject & owner, real radius) :
	TriggerVolume(owner, radius, NetworkTriggerVolumeNamespace::NetworkTriggerVolumeName, true)
{
}

//-----------------------------------------------------------------------

NetworkTriggerVolume::~NetworkTriggerVolume()
{
}

//-----------------------------------------------------------------------

bool NetworkTriggerVolume::isNetworkTriggerVolume() const
{
	return true;
}
	
//-----------------------------------------------------------------------

void NetworkTriggerVolume::virtualOnEnter(ServerObject& object)
{
	// we never observe ourself due to trigger volumes
	if (&object == &getOwner())
		return;

	Client *client = object.getClient();
	if (client)
		ObserveTracker::onClientEnteredNetworkTriggerVolume(*client, *this);
}

//-----------------------------------------------------------------------

void NetworkTriggerVolume::virtualOnExit(ServerObject& object)
{
	Client *client = object.getClient();
	if (client)
		ObserveTracker::onClientExitedNetworkTriggerVolume(*client, *this);
}

//-----------------------------------------------------------------------
