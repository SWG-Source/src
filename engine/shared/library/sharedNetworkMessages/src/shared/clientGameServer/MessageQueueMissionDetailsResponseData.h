//======================================================================
//
// MessageQueueMissionDetailsResponseData.h
// copyright (c) 2002 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_MessageQueueMissionDetailsResponseData_H
#define INCLUDED_MessageQueueMissionDetailsResponseData_H

//======================================================================

#include "sharedMath/Vector.h"
#include "sharedFoundation/NetworkId.h"
#include "StringId.h"

class Token;

struct MessageQueueMissionDetailsResponseData
{
	struct TokenData
	{
		Unicode::String  objectName;
		StringId         objectStringId;
		std::string      targetTemplateName;
		Vector           location;
		std::string      planetName;
	};

	MessageQueueMissionDetailsResponseData ();

	std::string type;

	NetworkId   idMissionData;

	//NetworkId   idOwner;
	TokenData     tokenOwner;

	//-- this NetworkId must be the NetworkId of a _TOKEN_
	//NetworkId   idTarget;
	TokenData     tokenTarget;

	//-- these NetworkIds can be the NetworkIds of _TOKENS_ or _WAYPOINTS_
	//NetworkId   idStart;
	TokenData   tokenStart;
	//NetworkId   idEnd;
	TokenData   tokenEnd;

	//-- zero targetCount indicates that the target is unique
	//-- non-zero targetCount indicates a number of 'similar' targets
	uint32      targetCount;

	uint32      reward;
	uint32      bond;

	//-- this is an absolute point in time
	uint32      timeExpiration;
	//-- this is an elapsed time segment, not absolute point in time
	uint32      timeLimit;

	//-- an empty profession indicates no rank necessary
	std::string profession;
	uint32      rankMinimum;

	//-- in percent units
	uint8       successRateMinimum;

	bool        friendsList;
	TokenData   tokenPlayerAssociation;
	//NetworkId   playerAssociation;

	//-- an empty faction indicates no factional requirement
	std::string factionName;
	
	//-- if factionName is nonempty, false indicates a sympathizer is required
	bool        factionDeclared;
};

//======================================================================

#endif
