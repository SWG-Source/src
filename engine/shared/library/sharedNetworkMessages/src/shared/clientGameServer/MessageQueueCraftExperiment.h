//========================================================================
//
// MessageQueueCraftExperiment.h
//
// copyright 2001 Sony Online Entertainment, Inc.
// All rights reserved.
//
//========================================================================


#ifndef INCLUDED_MessageQueueCraftExperiment_H
#define INCLUDED_MessageQueueCraftExperiment_H

#include "sharedFoundation/MessageQueue.h"
#include "sharedNetworkMessages/ControllerMessageMacros.h"

class MemoryBlockManager;

class MessageQueueCraftExperiment : public MessageQueue::Data
{
	CONTROLLER_MESSAGE_INTERFACE;

public:

	struct ExperimentInfo
	{
		int	attributeIndex;		// which attribute is being experiemnted
		int   experimentPoints;	// how many experiment points are allocated

		ExperimentInfo(int attribute, int points) :
			attributeIndex(attribute),
			experimentPoints(points)
		{
		}

		//private:
		//	ExperimentInfo & operator = (const ExperimentInfo &);
	};

public:

	         MessageQueueCraftExperiment(uint8 sequenceId);
	virtual ~MessageQueueCraftExperiment();

	void                                addExperiment(int attribute, int points);
	const std::vector<ExperimentInfo> & getExperiments(void) const;
	int                                 getTotalPoints(void) const;
	uint8                               getSequenceId(void) const;
	void								setCoreLevel(int level);
	int									getCoreLevel(void) const;

private:
	std::vector<ExperimentInfo> m_experiments;
	int                         m_totalPoints;
	uint8                       m_sequenceId;
	int							m_coreLevel;
};


inline MessageQueueCraftExperiment::MessageQueueCraftExperiment(uint8 sequenceId) :
	m_totalPoints(0),
	m_sequenceId(sequenceId),
	m_coreLevel(0)
{
}	// MessageQueueCraftExperiment::MessageQueueCraftExperiment()

inline void MessageQueueCraftExperiment::addExperiment(int attribute, int points)
{
	m_experiments.push_back(ExperimentInfo(attribute, points));
	m_totalPoints += points;
}	// MessageQueueCraftExperiment::addIngredient

inline const std::vector<MessageQueueCraftExperiment::ExperimentInfo> & 
	MessageQueueCraftExperiment::getExperiments(void) const
{
	return m_experiments;
}	// MessageQueueCraftExperiment::getExperiments

inline int MessageQueueCraftExperiment::getTotalPoints(void) const
{
	return m_totalPoints;
}	// MessageQueueCraftExperiment::getTotalPoints

inline uint8 MessageQueueCraftExperiment::getSequenceId(void) const
{
	return m_sequenceId;
}	// MessageQueueCraftExperiment::getSequenceId

inline void MessageQueueCraftExperiment::setCoreLevel(int level)
{
	m_coreLevel = level;
}

inline int MessageQueueCraftExperiment::getCoreLevel() const
{
	return m_coreLevel;
}
#endif	// INCLUDED_MessageQueueCraftExperiment_H
