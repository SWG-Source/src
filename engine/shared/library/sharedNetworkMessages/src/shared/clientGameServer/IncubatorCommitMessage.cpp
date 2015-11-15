// ======================================================================
//
// IncubatorCommitMessage.cpp
//
// Copyright 2007 Sony Online Entertainment
//
// ======================================================================

#include "sharedNetworkMessages/FirstSharedNetworkMessages.h"
#include "sharedNetworkMessages/IncubatorCommitMessage.h"

#include "sharedFoundation/ExitChain.h"
#include "sharedFoundation/GameControllerMessage.h"
#include "sharedFoundation/MemoryBlockManager.h"
#include "sharedNetworkMessages/ControllerMessageFactory.h"

// ======================================================================

CONTROLLER_MESSAGE_ALLOW_FROM_CLIENT_IMPLEMENTATION(IncubatorCommitMessage, CM_incubatorCommit);

//----------------------------------------------------------------------

IncubatorCommitMessage::IncubatorCommitMessage() :
MessageQueue::Data(),
m_committed(true),
m_terminalId(),
m_slot1Id(),
m_slot2Id(),
m_slot3Id(),
m_slot4Id(),
m_initialPointsSurvival(0),
m_initialPointsBeastialResilience(0),
m_initialPointsCunning(0),
m_initialPointsIntelligence(0),
m_initialPointsAggression(0),
m_initialPointsHuntersInstinct(0),
m_totalPointsSurvival(0),
m_totalPointsBeastialResilience(0),
m_totalPointsCunning(0),
m_totalPointsIntelligence(0),
m_totalPointsAggression(0),
m_totalPointsHuntersInstinct(0),
m_temperatureGauge(0),
m_nutrientGauge(0),
m_newCreatureColorIndex(0)
{
	
}

IncubatorCommitMessage::IncubatorCommitMessage(
	bool committed, 
	NetworkId const & terminalId,
	NetworkId const & slot1Id,
	NetworkId const & slot2Id,
	NetworkId const & slot3Id,
	NetworkId const & slot4Id,
	int initialPointsSurvival,
		int initialPointsBeastialResilience,
		int initialPointsCunning,
		int initialPointsIntelligence,
		int initialPointsAggression,
		int initialPointsHuntersInstinct,
		int totalPointsSurvival,
		int totalPointsBeastialResilience,
		int totalPointsCunning,
		int totalPointsIntelligence,
		int totalPointsAggression,
		int totalPointsHuntersInstinct,
		int temperatureGauge,
		int nutrientGauge,
		int newCreatureColorIndex
) :
MessageQueue::Data(),
m_committed(committed),
m_terminalId(terminalId),
m_slot1Id(slot1Id),
m_slot2Id(slot2Id),
m_slot3Id(slot3Id),
m_slot4Id(slot4Id),
m_initialPointsSurvival(initialPointsSurvival),
m_initialPointsBeastialResilience(initialPointsBeastialResilience),
m_initialPointsCunning(initialPointsCunning),
m_initialPointsIntelligence(initialPointsIntelligence),
m_initialPointsAggression(initialPointsAggression),
m_initialPointsHuntersInstinct(initialPointsHuntersInstinct),
m_totalPointsSurvival(totalPointsSurvival),
m_totalPointsBeastialResilience(totalPointsBeastialResilience),
m_totalPointsCunning(totalPointsCunning),
m_totalPointsIntelligence(totalPointsIntelligence),
m_totalPointsAggression(totalPointsAggression),
m_totalPointsHuntersInstinct(totalPointsHuntersInstinct),
m_temperatureGauge(temperatureGauge),
m_nutrientGauge(nutrientGauge),
m_newCreatureColorIndex(newCreatureColorIndex)
{
}

//----------------------------------------------------------------------

void IncubatorCommitMessage::setTerminalId(NetworkId const& val)
{
	m_terminalId = val;
}

//----------------------------------------------------------------------

void IncubatorCommitMessage::setSlot1Id(NetworkId const& val)
{
	m_slot1Id = val;
}

//----------------------------------------------------------------------

void IncubatorCommitMessage::setSlot2Id(NetworkId const& val)
{
	m_slot2Id = val;
}

//----------------------------------------------------------------------

void IncubatorCommitMessage::setSlot3Id(NetworkId const& val)
{
	m_slot3Id = val;
}

//----------------------------------------------------------------------

void IncubatorCommitMessage::setSlot4Id(NetworkId const& val)
{
	m_slot4Id = val;
}

//----------------------------------------------------------------------

void IncubatorCommitMessage::setCommitted(bool val)
{
	m_committed = val;
}

//----------------------------------------------------------------------

void IncubatorCommitMessage::setInitialPointsSurvival(int val)
{
	m_initialPointsSurvival = val;
}

//----------------------------------------------------------------------

void IncubatorCommitMessage::setInitialPointsBeastialResilience(int val)
{
	m_initialPointsBeastialResilience = val;
}

//----------------------------------------------------------------------

void IncubatorCommitMessage::setInitialPointsCunning(int val)
{
	m_initialPointsCunning = val;
}

//----------------------------------------------------------------------

void IncubatorCommitMessage::setInitialPointsIntelligence(int val)
{
	m_initialPointsIntelligence = val;
}

//----------------------------------------------------------------------

void IncubatorCommitMessage::setInitialPointsAggression(int val)
{
	m_initialPointsAggression = val;
}

//----------------------------------------------------------------------

void IncubatorCommitMessage::setInitialPointsHuntersInstinct(int val)
{
	m_initialPointsHuntersInstinct = val;
}

//----------------------------------------------------------------------

void IncubatorCommitMessage::setTotalPointsSurvival(int val)
{
	m_totalPointsSurvival = val;
}

//----------------------------------------------------------------------

void IncubatorCommitMessage::setTotalPointsBeastialResilience(int val)
{
	m_totalPointsBeastialResilience = val;
}

//----------------------------------------------------------------------

void IncubatorCommitMessage::setTotalPointsCunning(int val)
{
	m_totalPointsCunning = val;
}

//----------------------------------------------------------------------

void IncubatorCommitMessage::setTotalPointsIntelligence(int val)
{
	m_totalPointsIntelligence = val;
}

//----------------------------------------------------------------------

void IncubatorCommitMessage::setTotalPointsAggression(int val)
{
	m_totalPointsAggression = val;
}

//----------------------------------------------------------------------

void IncubatorCommitMessage::setTotalPointsHuntersInstinct(int val)
{
	m_totalPointsHuntersInstinct = val;
}

//----------------------------------------------------------------------

void IncubatorCommitMessage::setTemperatureGauge(int val)
{
	m_temperatureGauge = val;
}

//----------------------------------------------------------------------

void IncubatorCommitMessage::setNutrientGauge(int val)
{
	m_nutrientGauge = val;
}

void IncubatorCommitMessage::setNewCreatureColorIndex(int val)
{
	m_newCreatureColorIndex = val;
}

//----------------------------------------------------------------------

void IncubatorCommitMessage::pack(const MessageQueue::Data * const data, Archive::ByteStream & target)
{
	IncubatorCommitMessage const * const message = safe_cast<IncubatorCommitMessage const *> (data);
	if(message)
	{
		Archive::put(target, message->getCommitted());
		Archive::put(target, message->getTerminalId());
		Archive::put(target, message->getSlot1Id());
		Archive::put(target, message->getSlot2Id());
		Archive::put(target, message->getSlot3Id());
		Archive::put(target, message->getSlot4Id());
		Archive::put(target,message->getInitialPointsSurvival());
		Archive::put(target,message->getInitialPointsBeastialResilience());
		Archive::put(target,message->getInitialPointsCunning());
		Archive::put(target,message->getInitialPointsIntelligence());
		Archive::put(target,message->getInitialPointsAggression());
		Archive::put(target,message->getInitialPointsHuntersInstinct());
		Archive::put(target,message->getTotalPointsSurvival());
		Archive::put(target,message->getTotalPointsBeastialResilience());
		Archive::put(target,message->getTotalPointsCunning());
		Archive::put(target,message->getTotalPointsIntelligence());
		Archive::put(target,message->getTotalPointsAggression());
		Archive::put(target,message->getTotalPointsHuntersInstinct());
		Archive::put(target,message->getTemperatureGauge());
		Archive::put(target,message->getNutrientGauge());
		Archive::put(target,message->getNewCreatureColorIndex());
	}
}

//-----------------------------------------------------------------------

MessageQueue::Data * IncubatorCommitMessage::unpack(Archive::ReadIterator & source)
{
	NetworkId terminalId;
	NetworkId slot1Id;
	NetworkId slot2Id;
	NetworkId slot3Id;
	NetworkId slot4Id;
	bool committed;

	int initialPointsSurvival;
	int initialPointsBeastialResilience;
	int initialPointsCunning;
	int initialPointsIntelligence;
	int initialPointsAggression;
	int initialPointsHuntersInstinct;
	int totalPointsSurvival;
	int totalPointsBeastialResilience;
	int totalPointsCunning;
	int totalPointsIntelligence;
	int totalPointsAggression;
	int totalPointsHuntersInstinct;
	int temperatureGauge;
	int nutrientGauge;
	int newCreatureColorIndex;
	

	Archive::get(source, committed);
	Archive::get(source, terminalId);
	Archive::get(source, slot1Id);
	Archive::get(source, slot2Id);
	Archive::get(source, slot3Id);
	Archive::get(source, slot4Id);

	Archive::get(source, initialPointsSurvival);
	Archive::get(source, initialPointsBeastialResilience);
	Archive::get(source, initialPointsCunning);
	Archive::get(source, initialPointsIntelligence);
	Archive::get(source, initialPointsAggression);
	Archive::get(source, initialPointsHuntersInstinct);
	Archive::get(source, totalPointsSurvival);
	Archive::get(source, totalPointsBeastialResilience);
	Archive::get(source, totalPointsCunning);
	Archive::get(source, totalPointsIntelligence);
	Archive::get(source, totalPointsAggression);
	Archive::get(source, totalPointsHuntersInstinct);
	Archive::get(source, temperatureGauge);
	Archive::get(source, nutrientGauge);
	Archive::get(source, newCreatureColorIndex);
	
	return new IncubatorCommitMessage(
		committed, 
		terminalId,
		slot1Id,
		slot2Id,
		slot3Id,
		slot4Id,
		initialPointsSurvival,
		initialPointsBeastialResilience,
		initialPointsCunning,
		initialPointsIntelligence,
		initialPointsAggression,
		initialPointsHuntersInstinct,
		totalPointsSurvival,
		totalPointsBeastialResilience,
		totalPointsCunning,
		totalPointsIntelligence,
		totalPointsAggression,
		totalPointsHuntersInstinct,
		temperatureGauge,
		nutrientGauge,
		newCreatureColorIndex
	);
}

//----------------------------------------------------------------------
