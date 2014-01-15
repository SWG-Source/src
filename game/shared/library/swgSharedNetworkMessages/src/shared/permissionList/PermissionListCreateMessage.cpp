// ======================================================================
//
// PermissionListCreateMessage.cpp
// copyright (c) 2002 Sony Online Entertainment
//
// ======================================================================

#include "swgSharedNetworkMessages/FirstSwgSharedNetworkMessages.h"
#include "swgSharedNetworkMessages/PermissionListCreateMessage.h"

#include <vector>

// ======================================================================

const char * const PermissionListCreateMessage::MessageType = "PermissionListCreateMessage";

//----------------------------------------------------------------------

PermissionListCreateMessage::PermissionListCreateMessage (const std::vector<Unicode::String> &currentMembers, const std::vector<Unicode::String> &nearbyPeople, Unicode::String listName)
: GameNetworkMessage(MessageType),
  m_currentMembers(),
  m_nearbyPeople(),
  m_listName()
{
	m_currentMembers.set(currentMembers);
	m_nearbyPeople.set(nearbyPeople);
	m_listName.set(listName);
	addVariable(m_currentMembers);
	addVariable(m_nearbyPeople);
	addVariable(m_listName);
}

//-----------------------------------------------------------------------

PermissionListCreateMessage::PermissionListCreateMessage(Archive::ReadIterator & source)
: GameNetworkMessage(MessageType),
  m_currentMembers(),
  m_nearbyPeople(),
  m_listName()
{
	addVariable(m_currentMembers);
	addVariable(m_nearbyPeople);
	addVariable(m_listName);
	unpack(source);
}

//----------------------------------------------------------------------

PermissionListCreateMessage::~PermissionListCreateMessage()
{
}

//----------------------------------------------------------------------

const std::vector<Unicode::String>& PermissionListCreateMessage::getCurrentMembers () const
{
	return m_currentMembers.get();
}

//----------------------------------------------------------------------

const std::vector<Unicode::String>& PermissionListCreateMessage::getNearbyPeople () const
{
	return m_nearbyPeople.get();
}

//----------------------------------------------------------------------

const Unicode::String&                 PermissionListCreateMessage::getListName () const
{
	return m_listName.get();
}

// ======================================================================
