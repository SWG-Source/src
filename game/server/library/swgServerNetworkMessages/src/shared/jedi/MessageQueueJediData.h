//========================================================================
//
// MessageQueueJediData.h
//
// copyright 2001 Sony Online Entertainment
//
//========================================================================


#ifndef INCLUDED_MessageQueueJediData_H
#define INCLUDED_MessageQueueJediData_H

#include "sharedFoundation/MessageQueue.h"
#include "sharedFoundation/NetworkId.h"
#include "sharedMath/Vector.h"
#include "sharedNetworkMessages/ControllerMessageMacros.h"


class MemoryBlockManager;


/**
 */
class MessageQueueJediData : public MessageQueue::Data
{
	CONTROLLER_MESSAGE_INTERFACE;

public:
	MessageQueueJediData(const NetworkId & id, const Unicode::String & name, const Vector & location, const std::string & scene, int visibility, int bountyValue, int level, int hoursAlive, int state, int spentJediSkillPoints, int faction);
	virtual ~MessageQueueJediData();
	
	MessageQueueJediData&	operator=	(const MessageQueueJediData & source);
	MessageQueueJediData(const MessageQueueJediData & source);

	const NetworkId &              getId() const;
	const Unicode::String &        getName() const;
	const Vector &                 getLocation() const;
	const std::string &            getScene() const;
	int                            getVisibility() const;
	int                            getBountyValue() const;
	int                            getLevel() const;
	int                            getHoursAlive() const;
	int                            getState() const;
	int                            getSpentJediSkillPoints() const;
	int                            getFaction() const;
	
private:
	const NetworkId              m_id;
	const Unicode::String        m_name;
	const Vector                 m_location;
	const std::string            m_scene;
	const int                    m_visibility;
	const int                    m_bountyValue;
	const int                    m_level;
	const int                    m_hoursAlive;
	const int                    m_state;
	const int                    m_spentJediSkillPoints;
	const int                    m_faction;
};

// ======================================================================

inline const NetworkId & MessageQueueJediData::getId() const
{
	return m_id;
}

inline const Unicode::String & MessageQueueJediData::getName() const
{
	return m_name;
}

inline const Vector & MessageQueueJediData::getLocation() const
{
	return m_location;
}

inline const std::string & MessageQueueJediData::getScene() const
{
	return m_scene;
}

inline int MessageQueueJediData::getVisibility() const
{
	return m_visibility;
}

inline int MessageQueueJediData::getBountyValue() const
{
	return m_bountyValue;
}

inline int MessageQueueJediData::getLevel() const
{
	return m_level;
}

inline int MessageQueueJediData::getHoursAlive() const
{
	return m_hoursAlive;
}

inline int MessageQueueJediData::getState() const
{
	return m_state;
}

inline int MessageQueueJediData::getSpentJediSkillPoints() const
{
	return m_spentJediSkillPoints;
}

inline int MessageQueueJediData::getFaction() const
{
	return m_faction;
}

// ======================================================================


#endif	// INCLUDED_MessageQueueJediData_H
