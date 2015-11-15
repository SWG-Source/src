// ======================================================================
//
// StatMigrationTargetsMessage.cpp
//
// Copyright 2002 Sony Online Entertainment
//
// ======================================================================

#include "sharedNetworkMessages/FirstSharedNetworkMessages.h"
#include "sharedNetworkMessages/StatMigrationTargetsMessage.h"
#include "swgSharedUtility/Attributes.def"

// ======================================================================

const char* const StatMigrationTargetsMessage::cms_name = "StatMigrationTargetsMessage";

// ======================================================================

StatMigrationTargetsMessage::StatMigrationTargetsMessage(const std::vector<int>& currentTargets, int pointsLeft)
: GameNetworkMessage(cms_name),
  m_health(0),
  m_constitution(0),
  m_action(0),
  m_stamina(0),
  m_mind(0),
  m_willpower(0),
  m_pointsLeft(pointsLeft)
{
	DEBUG_FATAL(static_cast<int>(currentTargets.size()) != Attributes::NumberOfAttributes, ("Bad number of targets given when building a StatMigrationTargetsMessage"));

	m_health.set(currentTargets[Attributes::Health]);
	m_constitution.set(currentTargets[Attributes::Constitution]);
	m_action.set(currentTargets[Attributes::Action]);
	m_stamina.set(currentTargets[Attributes::Stamina]);
	m_mind.set(currentTargets[Attributes::Mind]);
	m_willpower.set(currentTargets[Attributes::Willpower]);

	AutoByteStream::addVariable(m_health);
	AutoByteStream::addVariable(m_constitution);
	AutoByteStream::addVariable(m_action);
	AutoByteStream::addVariable(m_stamina);
	AutoByteStream::addVariable(m_mind);
	AutoByteStream::addVariable(m_willpower);
	AutoByteStream::addVariable(m_pointsLeft);
}

// ======================================================================

StatMigrationTargetsMessage::StatMigrationTargetsMessage(Archive::ReadIterator &source)
: GameNetworkMessage(cms_name),
  m_health(0),
  m_constitution(0),
  m_action(0),
  m_stamina(0),
  m_mind(0),
  m_willpower(0),
  m_pointsLeft(0)
{
	AutoByteStream::addVariable(m_health);
	AutoByteStream::addVariable(m_constitution);
	AutoByteStream::addVariable(m_action);
	AutoByteStream::addVariable(m_stamina);
	AutoByteStream::addVariable(m_mind);
	AutoByteStream::addVariable(m_willpower);
	AutoByteStream::addVariable(m_pointsLeft);
	unpack(source);
}

// ----------------------------------------------------------------------

std::vector<int> StatMigrationTargetsMessage::getTargets() const
{
	std::vector<int> vec;
	vec.push_back(m_health.get());
	vec.push_back(m_constitution.get());
	vec.push_back(m_action.get());
	vec.push_back(m_stamina.get());
	vec.push_back(m_mind.get());
	vec.push_back(m_willpower.get());
	return vec;
}

// ----------------------------------------------------------------------

StatMigrationTargetsMessage::~StatMigrationTargetsMessage()
{
}

// ======================================================================
