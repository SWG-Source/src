// ======================================================================
//
// IncubatorStartMessage.cpp
//
// Copyright 2007 Sony Online Entertainment
//
// ======================================================================

#include "sharedNetworkMessages/FirstSharedNetworkMessages.h"
#include "sharedNetworkMessages/IncubatorStartMessage.h"

#include "sharedFoundation/ExitChain.h"
#include "sharedFoundation/GameControllerMessage.h"
#include "sharedFoundation/MemoryBlockManager.h"
#include "sharedNetworkMessages/ControllerMessageFactory.h"

#include <cstdio>

// ======================================================================

CONTROLLER_MESSAGE_IMPLEMENTATION(IncubatorStartMessage, CM_incubatorStart);

//----------------------------------------------------------------------

IncubatorStartMessage::IncubatorStartMessage(
		int sessionNumber,
		NetworkId const & playerId, 
		NetworkId const & terminalId,
		int powerGauge,
		int initialPointsSurvival,
		int initialPointsBeastialResilience,
		int initialPointsCunning,
		int initialPointsIntelligence,
		int initialPointsAggression,
		int initialPointsHuntersInstinct,
		int temperatureGauge,
		int nutrientGauge,
		int initialCreatureColorIndex,
		std::string const& creatureTemplateName
		) :
MessageQueue::Data(),
m_sessionNumber(sessionNumber),
m_playerId(playerId),
m_terminalId(terminalId),
m_powerGauge(powerGauge),
m_initialPointsSurvival(initialPointsSurvival),
m_initialPointsBeastialResilience(initialPointsBeastialResilience),
m_initialPointsCunning(initialPointsCunning),
m_initialPointsIntelligence(initialPointsIntelligence),
m_initialPointsAggression(initialPointsAggression),
m_initialPointsHuntersInstinct(initialPointsHuntersInstinct),
m_temperatureGauge(temperatureGauge),
m_nutrientGauge(nutrientGauge),
m_initialCreatureColorIndex(initialCreatureColorIndex),
m_creatureTemplateName(creatureTemplateName)
{
}

//----------------------------------------------------------------------

void IncubatorStartMessage::pack(const MessageQueue::Data * const data, Archive::ByteStream & target)
{
	IncubatorStartMessage const * const message = safe_cast<IncubatorStartMessage const *> (data);
	if(message)
	{
		Archive::put(target, message->getSessionNumber());
		Archive::put(target, message->getPlayerId());
		Archive::put(target, message->getTerminalId());
		Archive::put(target, message->getPowerGauge());
		Archive::put(target, message->getInitialPointsSurvival());
		Archive::put(target, message->getInitialPointsBeastialResilience());
		Archive::put(target, message->getInitialPointsCunning());
		Archive::put(target, message->getInitialPointsIntelligence());
		Archive::put(target, message->getInitialPointsAggression());
		Archive::put(target, message->getInitialPointsHuntersInstinct());
		Archive::put(target, message->getTemperatureGauge());
		Archive::put(target, message->getNutrientGauge() );
		Archive::put(target, message->getInitialCreatureColorIndex() );
		Archive::put(target, message->getCreatureTemplateName());
	}
}

//-----------------------------------------------------------------------

MessageQueue::Data * IncubatorStartMessage::unpack(Archive::ReadIterator & source)
{
	int sessionNumber; 
	NetworkId playerId;
	NetworkId terminalId;
	int powerGauge;
	int initialPointsSurvival;
	int initialPointsBeastialResilience;
	int initialPointsCunning;
	int initialPointsIntelligence;
	int initialPointsAggression;
	int initialPointsHuntersInstinct;
	int temperatureGauge; 
	int nutrientGauge; 
	int initialCreatureColorIndex;
	std::string creatureTemplateName;
	
	Archive::get(source, sessionNumber);
	Archive::get(source, playerId);
	Archive::get(source, terminalId);
	Archive::get(source, powerGauge);
	Archive::get(source, initialPointsSurvival);
	Archive::get(source, initialPointsBeastialResilience);
	Archive::get(source, initialPointsCunning);
	Archive::get(source, initialPointsIntelligence);
	Archive::get(source, initialPointsAggression);
	Archive::get(source, initialPointsHuntersInstinct);
	Archive::get(source, temperatureGauge);
	Archive::get(source, nutrientGauge);
	Archive::get(source, initialCreatureColorIndex);
	Archive::get(source, creatureTemplateName);
	
	return new IncubatorStartMessage(
		sessionNumber,
		playerId, 
		terminalId,
		powerGauge,
		initialPointsSurvival,
		initialPointsBeastialResilience,
		initialPointsCunning,
		initialPointsIntelligence,
		initialPointsAggression,
		initialPointsHuntersInstinct,
		temperatureGauge,
		nutrientGauge,
		initialCreatureColorIndex,
		creatureTemplateName
	);
}

//----------------------------------------------------------------------

void IncubatorStartMessage::getStringRepresentation(std::string& buff) const
{
	char tmp[512];

	sprintf(tmp,
		"%d %s %s %d %d %d %d %d %d %d %d %d %d %s ",
		getSessionNumber(),
		getPlayerId().getValueString().c_str(),
		getTerminalId().getValueString().c_str(),
		getPowerGauge(),
		getInitialPointsSurvival(),
		getInitialPointsBeastialResilience(),
		getInitialPointsCunning(),
		getInitialPointsIntelligence(),
		getInitialPointsAggression(),
		getInitialPointsHuntersInstinct(),
		getTemperatureGauge(),
		getNutrientGauge(),
		getInitialCreatureColorIndex(), 
		getCreatureTemplateName().c_str()
	);

	buff = tmp;
}

//----------------------------------------------------------------------
