//======================================================================
//
// MessageQueueMissionDetailsResponseDataArchive.cpp
// copyright (c) 2002 Sony Online Entertainment
//
//======================================================================

#include "sharedNetworkMessages/FirstSharedNetworkMessages.h"
#include "sharedNetworkMessages/MessageQueueMissionDetailsResponseDataArchive.h"

#include "localizationArchive/StringIdArchive.h"
#include "sharedMathArchive/VectorArchive.h"
#include "sharedNetworkMessages/MessageQueueMissionDetailsResponseData.h"

//======================================================================

namespace Archive
{
	void get (ReadIterator & source, MessageQueueMissionDetailsResponseData::TokenData & target)
	{
		get(source, target.objectName);
		get(source, target.objectStringId);
		get(source, target.targetTemplateName);
		get(source, target.location);
		get(source, target.planetName);
	}

	void put (ByteStream & target, const MessageQueueMissionDetailsResponseData::TokenData & source)
	{
		put(target, source.objectName);
		put(target, source.objectStringId);
		put(target, source.targetTemplateName);
		put(target, source.location);
		put(target, source.planetName);
	}

	void get (ReadIterator & source, MessageQueueMissionDetailsResponseData & target)
	{
		get (source, target.type);		
		get (source, target.idMissionData);
//		get (source, target.idOwner);
		get (source, target.tokenOwner);

//		get (source, target.idTarget);
		get (source, target.tokenTarget);

//		get (source, target.idStart);
		get (source, target.tokenStart);

//		get (source, target.idEnd);
		get (source, target.tokenEnd);

		get (source, target.targetCount);
		get (source, target.reward);
		get (source, target.bond);
		get (source, target.timeExpiration);
		get (source, target.timeLimit);
		get (source, target.profession);
		get (source, target.rankMinimum);
		get (source, target.successRateMinimum);
		get (source, target.friendsList);
		//get (source, target.playerAssociation);
		get (source, target.tokenPlayerAssociation);
		get (source, target.factionName);
		get (source, target.factionDeclared);
	}

	void put (ByteStream & target, const MessageQueueMissionDetailsResponseData & source)
	{
		put (target, source.type);
		put (target, source.idMissionData);

		put (target, source.tokenOwner);
//		put (target, source.idOwner);

		put (target, source.tokenTarget);
//		put (target, source.idTarget);

		put (target, source.tokenStart);
//		put (target, source.idStart);

		put (target, source.tokenEnd);
//		put (target, source.idEnd);

		put (target, source.targetCount);
		put (target, source.reward);
		put (target, source.bond);
		put (target, source.timeExpiration);
		put (target, source.timeLimit);
		put (target, source.profession);
		put (target, source.rankMinimum);
		put (target, source.successRateMinimum);
		put (target, source.friendsList);

		put (target, source.tokenPlayerAssociation);
//		put (target, source.playerAssociation);

		put (target, source.factionName);
		put (target, source.factionDeclared);
	}
}
//======================================================================
