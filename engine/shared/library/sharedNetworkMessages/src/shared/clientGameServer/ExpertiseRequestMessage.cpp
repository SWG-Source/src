// ======================================================================
//
// ExpertiseRequestMessage.cpp
// Copyright 2002 Sony Online Entertainment
//
// ======================================================================

#include "sharedNetworkMessages/FirstSharedNetworkMessages.h"
#include "sharedNetworkMessages/ExpertiseRequestMessage.h"

// ======================================================================

ExpertiseRequestMessage::ExpertiseRequestMessage() :
	GameNetworkMessage("ExpertiseRequestMessage")
{
	addVariable(m_addExpertisesList);
	addVariable(m_clearAllExpertisesFirst);
}

// ----------------------------------------------------------------------

ExpertiseRequestMessage::ExpertiseRequestMessage(Archive::ReadIterator &source) :
	GameNetworkMessage("ExpertiseRequestMessage")
{
	addVariable(m_addExpertisesList);
	addVariable(m_clearAllExpertisesFirst);
	unpack(source);
}

// ----------------------------------------------------------------------

void ExpertiseRequestMessage::setAddExpertisesList(const std::vector<std::string> & addExpertiseList)
{
	m_addExpertisesList.set (addExpertiseList);
}

// ----------------------------------------------------------------------

void ExpertiseRequestMessage::setClearAllExpertisesFirst(bool clearAllExpertisesFirst)
{
	m_clearAllExpertisesFirst.set (clearAllExpertisesFirst);
}

// ----------------------------------------------------------------------

ExpertiseRequestMessage::~ExpertiseRequestMessage()
{
}

// ======================================================================
