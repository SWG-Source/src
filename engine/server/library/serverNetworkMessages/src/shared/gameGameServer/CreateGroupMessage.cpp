// ======================================================================
//
// CreateGroupMessage.cpp
//
// Copyright 2003 Sony Online Entertainment
//
// ======================================================================

#include "serverNetworkMessages/FirstServerNetworkMessages.h"
#include "serverNetworkMessages/CreateGroupMessage.h"

// ======================================================================

CreateGroupMessage::CreateGroupMessage(GroupMemberParam const & leader, std::vector<GroupMemberParam> const & members) :
	GameNetworkMessage("CreateGroupMessage"),
	m_leader(leader),
	m_members(members)
{
	addVariable(m_leader);
	addVariable(m_members);
}

// ----------------------------------------------------------------------

CreateGroupMessage::CreateGroupMessage(Archive::ReadIterator & source) :
	GameNetworkMessage("CreateGroupMessage"),
	m_leader(),
	m_members()
{
	addVariable(m_leader);
	addVariable(m_members);
	unpack(source);
}

// ----------------------------------------------------------------------

CreateGroupMessage::~CreateGroupMessage()
{
}

// ======================================================================

