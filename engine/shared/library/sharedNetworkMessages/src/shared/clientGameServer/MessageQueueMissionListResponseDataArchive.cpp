//======================================================================
//
// MessageQueueMissionListResponseDataArchive.cpp
// copyright (c) 2002 Sony Online Entertainment
//
//======================================================================

#include "sharedNetworkMessages/FirstSharedNetworkMessages.h"
#include "sharedNetworkMessages/MessageQueueMissionListResponseDataArchive.h"

#include "sharedNetworkMessages/MessageQueueMissionListResponseData.h"
#include "localizationArchive/StringIdArchive.h"

//======================================================================

namespace Archive
{
	void get (ReadIterator & source, MessageQueueMissionListResponseData & target)
	{
		Archive::get(source, target.bond);
		Archive::get(source, target.creatorName);
		Archive::get(source, target.targetName);
		Archive::get(source, target.description);
		Archive::get(source, target.difficulty);
		Archive::get(source, target.endLocationPlanetName);
		Archive::get(source, target.endLocationRegionName);
		Archive::get(source, target.missionData);
		Archive::get(source, target.missionType);
		Archive::get(source, target.reward);
		Archive::get(source, target.startLocationPlanetName);
		Archive::get(source, target.startLocationRegionName);
		Archive::get(source, target.title);
	}
	
	void put (ByteStream & target, const MessageQueueMissionListResponseData & source)
	{ 
		Archive::put(target, source.bond);
		Archive::put(target, source.creatorName);
		Archive::put(target, source.targetName);
		Archive::put(target, source.description);
		Archive::put(target, source.difficulty);
		Archive::put(target, source.endLocationPlanetName);
		Archive::put(target, source.endLocationRegionName);
		Archive::put(target, source.missionData);
		Archive::put(target, source.missionType);
		Archive::put(target, source.reward);
		Archive::put(target, source.startLocationPlanetName);
		Archive::put(target, source.startLocationRegionName);
		Archive::put(target, source.title);
	}
	
}

//======================================================================
