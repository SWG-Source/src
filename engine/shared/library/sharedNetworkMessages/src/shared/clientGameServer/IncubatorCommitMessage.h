// ======================================================================
//
// IncubatorCommitMessage.h
//
// Copyright 2007 Sony Online Entertainment
//
// ======================================================================

#ifndef	_IncubatorCommitMessage_H
#define	_IncubatorCommitMessage_H

//-----------------------------------------------------------------------
#include "sharedFoundation/MessageQueue.h"
#include "sharedNetworkMessages/ControllerMessageMacros.h"

//-----------------------------------------------------------------------

class IncubatorCommitMessage : public MessageQueue::Data
{
	CONTROLLER_MESSAGE_INTERFACE;

public:
	IncubatorCommitMessage();
	IncubatorCommitMessage(
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
		);

public:
	NetworkId const & getTerminalId() const;
	NetworkId const & getSlot1Id() const;
	NetworkId const & getSlot2Id() const;
	NetworkId const & getSlot3Id() const;
	NetworkId const & getSlot4Id() const;
	bool getCommitted() const;
	int getInitialPointsSurvival() const;
	int getInitialPointsBeastialResilience() const;
	int getInitialPointsCunning() const;
	int getInitialPointsIntelligence() const;
	int getInitialPointsAggression() const;
	int getInitialPointsHuntersInstinct() const;
	int getTotalPointsSurvival() const;
	int getTotalPointsBeastialResilience() const;
	int getTotalPointsCunning() const;
	int getTotalPointsIntelligence() const;
	int getTotalPointsAggression() const;
	int getTotalPointsHuntersInstinct() const;
	int getTemperatureGauge() const;
	int getNutrientGauge() const;
	int getNewCreatureColorIndex() const;

	void setTerminalId(NetworkId const& val);
	void setCommitted(bool val);
	void setSlot1Id(NetworkId const& val);
	void setSlot2Id(NetworkId const& val);
	void setSlot3Id(NetworkId const& val);
	void setSlot4Id(NetworkId const& val);
	void setInitialPointsSurvival(int val);
	void setInitialPointsBeastialResilience(int val);
	void setInitialPointsCunning(int val);
	void setInitialPointsIntelligence(int val);
	void setInitialPointsAggression(int val);
	void setInitialPointsHuntersInstinct(int val);
	void setTotalPointsSurvival(int val);
	void setTotalPointsBeastialResilience(int val);
	void setTotalPointsCunning(int val);
	void setTotalPointsIntelligence(int val);
	void setTotalPointsAggression(int val);
	void setTotalPointsHuntersInstinct(int val);
	void setTemperatureGauge(int val);
	void setNutrientGauge(int val);
	void setNewCreatureColorIndex(int val);

private:
//disabled
	IncubatorCommitMessage(IncubatorCommitMessage const & source);

private:
	bool m_committed;
	NetworkId m_terminalId;

	NetworkId m_slot1Id;
	NetworkId m_slot2Id;
	NetworkId m_slot3Id;
	NetworkId m_slot4Id;
	
	int m_initialPointsSurvival;
	int m_initialPointsBeastialResilience;
	int m_initialPointsCunning;
	int m_initialPointsIntelligence;
	int m_initialPointsAggression;
	int m_initialPointsHuntersInstinct;

	int m_totalPointsSurvival;
	int m_totalPointsBeastialResilience;
	int m_totalPointsCunning;
	int m_totalPointsIntelligence;
	int m_totalPointsAggression;
	int m_totalPointsHuntersInstinct;

	int m_temperatureGauge;
	int m_nutrientGauge;

	int m_newCreatureColorIndex;
};

// ----------------------------------------------------------------------

inline NetworkId const & IncubatorCommitMessage::getTerminalId() const
{
	return m_terminalId;
}

// ----------------------------------------------------------------------

inline NetworkId const & IncubatorCommitMessage::getSlot1Id() const
{
	return m_slot1Id;
}

// ----------------------------------------------------------------------

inline NetworkId const & IncubatorCommitMessage::getSlot2Id() const
{
	return m_slot2Id;
}

// ----------------------------------------------------------------------

inline NetworkId const & IncubatorCommitMessage::getSlot3Id() const
{
	return m_slot3Id;
}

// ----------------------------------------------------------------------

inline NetworkId const & IncubatorCommitMessage::getSlot4Id() const
{
	return m_slot4Id;
}

// ----------------------------------------------------------------------

inline bool IncubatorCommitMessage::getCommitted() const
{
	return m_committed;
}

// ----------------------------------------------------------------------

inline int IncubatorCommitMessage::getInitialPointsSurvival() const
{
	return m_initialPointsSurvival;
}

// ----------------------------------------------------------------------

inline int IncubatorCommitMessage::getInitialPointsBeastialResilience() const
{
	return m_initialPointsBeastialResilience;
}

// ----------------------------------------------------------------------

inline int IncubatorCommitMessage::getInitialPointsCunning() const
{
	return m_initialPointsCunning;
}

// ----------------------------------------------------------------------

inline int IncubatorCommitMessage::getInitialPointsIntelligence() const
{
	return m_initialPointsIntelligence;
}

// ----------------------------------------------------------------------

inline int IncubatorCommitMessage::getInitialPointsAggression() const
{
	return m_initialPointsAggression;
}

// ----------------------------------------------------------------------

inline int IncubatorCommitMessage::getInitialPointsHuntersInstinct() const
{
	return m_initialPointsHuntersInstinct;
}

// ----------------------------------------------------------------------

inline int IncubatorCommitMessage::getTotalPointsSurvival() const
{
	return m_totalPointsSurvival;
}

// ----------------------------------------------------------------------
inline int IncubatorCommitMessage::getTotalPointsBeastialResilience() const
{
	return m_totalPointsBeastialResilience;
}

// ----------------------------------------------------------------------
inline int IncubatorCommitMessage::getTotalPointsCunning() const
{
	return m_totalPointsCunning;
}

// ----------------------------------------------------------------------

inline int IncubatorCommitMessage::getTotalPointsIntelligence() const
{
	return m_totalPointsIntelligence;
}

// ----------------------------------------------------------------------
inline int IncubatorCommitMessage::getTotalPointsAggression() const
{
	return m_totalPointsAggression;
}

// ----------------------------------------------------------------------

inline int IncubatorCommitMessage::getTotalPointsHuntersInstinct() const
{
	return m_totalPointsHuntersInstinct;
}

// ----------------------------------------------------------------------

inline int IncubatorCommitMessage::getTemperatureGauge() const
{
	return m_temperatureGauge; // 0 -> 10
}

// ----------------------------------------------------------------------

inline int IncubatorCommitMessage::getNutrientGauge() const
{
	return m_nutrientGauge; // 0 -> 10
}

// ----------------------------------------------------------------------

inline int IncubatorCommitMessage::getNewCreatureColorIndex() const
{
	return m_newCreatureColorIndex;
}

// ----------------------------------------------------------------------
#endif 

