// ======================================================================
//
// PermissionListModifyMessage.cpp
// copyright (c) 2002 Sony Online Entertainment
//
// ======================================================================

#include "swgSharedNetworkMessages/FirstSwgSharedNetworkMessages.h"
#include "swgSharedNetworkMessages/PermissionListModifyMessage.h"

// ======================================================================

PermissionListModifyMessage::PermissionListModifyMessage (const Unicode::String& person, const Unicode::String& listName, const Unicode::String& action)
: GameNetworkMessage("PermissionListModifyMessage"),
  m_person  (),
  m_listName(),
  m_action  ()
{
	m_person.set  (person);
	m_listName.set(listName);
	m_action.set  (action);
	addVariable   (m_person);
	addVariable   (m_listName);
	addVariable   (m_action);
}

//-----------------------------------------------------------------------

PermissionListModifyMessage::PermissionListModifyMessage(Archive::ReadIterator & source)
: GameNetworkMessage("PermissionListModifyMessage"),
  m_person  (),
  m_listName(),
  m_action  ()
{
	addVariable(m_person);
	addVariable(m_listName);
	addVariable(m_action);
	unpack     (source);
}

//----------------------------------------------------------------------

PermissionListModifyMessage::~PermissionListModifyMessage()
{
}

//----------------------------------------------------------------------

const Unicode::String & PermissionListModifyMessage::getPerson () const
{
	return m_person.get();
}

//----------------------------------------------------------------------

const Unicode::String & PermissionListModifyMessage::getListName () const
{
	return m_listName.get();
}

//----------------------------------------------------------------------

const Unicode::String & PermissionListModifyMessage::getAction () const
{
	return m_action.get();
}

// =====================================================================
