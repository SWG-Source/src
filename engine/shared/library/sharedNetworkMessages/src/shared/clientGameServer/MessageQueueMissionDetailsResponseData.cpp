//======================================================================
//
// MessageQueueMissionDetailsResponseData.cpp
// copyright (c) 2002 Sony Online Entertainment
//
//======================================================================

#include "sharedNetworkMessages/FirstSharedNetworkMessages.h"
#include "sharedNetworkMessages/MessageQueueMissionDetailsResponseData.h"

//======================================================================

MessageQueueMissionDetailsResponseData::MessageQueueMissionDetailsResponseData() :
type (),
idMissionData (),
tokenOwner (),
tokenTarget (),
tokenStart (),
tokenEnd (),
targetCount (0),
reward (0),
bond (0),
timeExpiration (0),
timeLimit (0),
profession (),
rankMinimum (0),
successRateMinimum (0),
friendsList (false),
tokenPlayerAssociation (),
factionName (),
factionDeclared (false)
{
}

//======================================================================
