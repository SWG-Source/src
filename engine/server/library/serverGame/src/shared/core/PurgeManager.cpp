// ======================================================================
//
// PurgeManager.cpp
// copyright (c) 2005 Sony Online Entertainment
//
// ======================================================================

#include "serverGame/FirstServerGame.h"
#include "serverGame/PurgeManager.h"

#include "serverGame/GameServer.h"
#include "serverGame/MessageToQueue.h"
#include "sharedLog/Log.h"
#include "sharedNetworkMessages/GenericValueTypeMessage.h"
#include "UnicodeUtils.h"

// ======================================================================

/**
 * Send messages to all the listed structures and vendors telling them to purge themselves
 * @param stationId The stationId of the account initiating the purge
 * @param structures vector of (structure id, owner id) pairs
 * @param vendors vector of NetworkIds of vendors
 */
void PurgeManager::handleStructuresAndVendorsForPurge(StationId stationId, std::vector<std::pair<NetworkId, NetworkId> > const & structures, std::vector<std::pair<NetworkId, std::pair<NetworkId, Unicode::String> > > const & vendors, bool warnOnly)
{
	{
		std::string structuresString("[");
		for (std::vector<std::pair<NetworkId, NetworkId> >::const_iterator i=structures.begin(); i!=structures.end(); )
		{
			structuresString += i->first.getValueString() + " (owner " + i->second.getValueString()+")";
			if (++i!=structures.end())
				structuresString += ", ";
		}
		structuresString+="]";
	
		std::string vendorsString("[");
		for (std::vector<std::pair<NetworkId, std::pair<NetworkId, Unicode::String> > >::const_iterator j=vendors.begin(); j!=vendors.end(); )
		{
			vendorsString += j->first.getValueString() + " (owner " + j->second.first.getValueString()+")";
			if (++j!=vendors.end())
				vendorsString += ", ";
		}
		vendorsString+="]";
	
		LOG("CustomerService",("Purge:  account %li, structures for %s are %s, vendors for purge are %s",stationId, warnOnly ? "warning emails" : "purge", structuresString.c_str(), vendorsString.c_str()));
	}
	
	if (warnOnly)
	{
		for (std::vector<std::pair<NetworkId, NetworkId> >::const_iterator i=structures.begin(); i!=structures.end(); ++i)
			MessageToQueue::getInstance().sendMessageToC(i->first, "C++WarnAboutPurge", i->second.getValueString(), 0, true);

		for (std::vector<std::pair<NetworkId, std::pair<NetworkId, Unicode::String> > >::const_iterator j=vendors.begin(); j!=vendors.end(); ++j)
			MessageToQueue::getInstance().sendMessageToC(j->second.first, "C++WarnAboutVendorPurge", Unicode::wideToNarrow(j->second.second), 0, true);
	}
	else
	{
		for (std::vector<std::pair<NetworkId, NetworkId> >::const_iterator i=structures.begin(); i!=structures.end(); ++i)
			MessageToQueue::getInstance().sendMessageToC(i->first, "C++PurgeStructure", i->second.getValueString(), 0, true);

		for (std::vector<std::pair<NetworkId, std::pair<NetworkId, Unicode::String> > >::const_iterator j=vendors.begin(); j!=vendors.end(); ++j)
			MessageToQueue::getInstance().sendMessageToC(j->first, "C++PurgeVendor", "", 0, true);
	}
	
	// Now that the messages are sent, wait for a save cycle and then tell the login server that the purge finished.
	// Even if the structures weren't removed yet, we can assume that the persisted messageTos will have been saved.
	GenericValueTypeMessage<StationId> msg("PurgeCompleteMessage",stationId);
	GameServer::getInstance().sendToDatabaseServer(msg);
}

// ======================================================================
