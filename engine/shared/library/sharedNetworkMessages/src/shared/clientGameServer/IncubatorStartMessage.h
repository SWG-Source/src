// ======================================================================
//
// IncubatorStartMessage.h
//
// Copyright 2007 Sony Online Entertainment
//
// ======================================================================

#ifndef	_IncubatorStartMessage_H
#define	_IncubatorStartMessage_H

//-----------------------------------------------------------------------
#include "sharedFoundation/MessageQueue.h"
#include "sharedNetworkMessages/ControllerMessageMacros.h"

//-----------------------------------------------------------------------

class IncubatorStartMessage : public MessageQueue::Data
{
	CONTROLLER_MESSAGE_INTERFACE;

public:
	IncubatorStartMessage(
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
	);

public:
	int getSessionNumber() const;
	NetworkId const & getPlayerId() const;
	NetworkId const & getTerminalId() const;
	int getPowerGauge() const;
	int getInitialPointsSurvival() const;
	int getInitialPointsBeastialResilience() const;
	int getInitialPointsCunning() const;
	int getInitialPointsIntelligence() const;
	int getInitialPointsAggression() const;
	int getInitialPointsHuntersInstinct() const;
	int getTemperatureGauge() const;
	int getNutrientGauge() const;
	int getInitialCreatureColorIndex() const;
	std::string const& getCreatureTemplateName() const;

	void getStringRepresentation(std::string& buff) const;

private:
//disabled
	IncubatorStartMessage(IncubatorStartMessage const & source);

private:
	int m_sessionNumber; // 1-3
	NetworkId m_playerId;
	NetworkId m_terminalId;
	int m_powerGauge; // 0 -> 1000
	int m_initialPointsSurvival;
	int m_initialPointsBeastialResilience;
	int m_initialPointsCunning;
	int m_initialPointsIntelligence;
	int m_initialPointsAggression;
	int m_initialPointsHuntersInstinct;
	int m_temperatureGauge; // 0 -> 10
	int m_nutrientGauge; // 0 -> 10
	int m_initialCreatureColorIndex;
	std::string m_creatureTemplateName;
};

// ----------------------------------------------------------------------

inline NetworkId const & IncubatorStartMessage::getPlayerId() const
{
	return m_playerId;
}

// ----------------------------------------------------------------------

inline NetworkId const & IncubatorStartMessage::getTerminalId() const
{
	return m_terminalId;
}

// ----------------------------------------------------------------------

inline int IncubatorStartMessage::getSessionNumber() const
{
	return m_sessionNumber;
}

// ----------------------------------------------------------------------

inline int IncubatorStartMessage::getPowerGauge() const
{
	return m_powerGauge;
}

// ----------------------------------------------------------------------

inline int IncubatorStartMessage::getInitialPointsSurvival() const
{
	return m_initialPointsSurvival;
}

// ----------------------------------------------------------------------

inline int IncubatorStartMessage::getInitialPointsBeastialResilience() const
{
	return m_initialPointsBeastialResilience;
}

// ----------------------------------------------------------------------

inline int IncubatorStartMessage::getInitialPointsCunning() const
{
	return m_initialPointsCunning;
}

// ----------------------------------------------------------------------

inline int IncubatorStartMessage::getInitialPointsIntelligence() const
{
	return m_initialPointsIntelligence;
}

// ----------------------------------------------------------------------

inline int IncubatorStartMessage::getInitialPointsAggression() const
{
	return m_initialPointsAggression;
}

// ----------------------------------------------------------------------

inline int IncubatorStartMessage::getInitialPointsHuntersInstinct() const
{
	return m_initialPointsHuntersInstinct;
}

// ----------------------------------------------------------------------

inline int IncubatorStartMessage::getTemperatureGauge() const
{
	return m_temperatureGauge; // 0 -> 10
}

// ----------------------------------------------------------------------

inline int IncubatorStartMessage::getNutrientGauge() const
{
	return m_nutrientGauge; // 0 -> 10
}

// ----------------------------------------------------------------------

inline std::string const& IncubatorStartMessage::getCreatureTemplateName() const
{
	return m_creatureTemplateName;
}

// ----------------------------------------------------------------------

inline int IncubatorStartMessage::getInitialCreatureColorIndex() const
{
	return m_initialCreatureColorIndex;
}

// ----------------------------------------------------------------------

#endif 

