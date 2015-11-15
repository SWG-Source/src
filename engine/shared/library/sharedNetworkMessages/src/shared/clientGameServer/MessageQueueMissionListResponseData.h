//======================================================================
//
// MessageQueueMissionListResponseData.h
// copyright (c) 2002 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_MessageQueueMissionListResponseData_H
#define INCLUDED_MessageQueueMissionListResponseData_H

//======================================================================

#include "sharedFoundation/NetworkId.h"
#include "StringId.h"

//----------------------------------------------------------------------

class MessageQueueMissionListResponseData
{
public:
	int              bond;
	Unicode::String  creatorName;
	Unicode::String  targetName;
	StringId         description;
	int              difficulty;
	std::string      endLocationPlanetName;
	Unicode::String  endLocationRegionName;
	NetworkId        missionData;
	Unicode::String  missionType;
	int              reward;
	std::string      startLocationPlanetName;
	Unicode::String  startLocationRegionName;
	StringId         title;
};

//======================================================================

#endif
