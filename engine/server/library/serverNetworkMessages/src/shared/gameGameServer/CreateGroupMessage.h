// ======================================================================
//
// CreateGroupMessage.h
//
// Copyright 2003 Sony Online Entertainment
//
// ======================================================================

#ifndef	_INCLUDED_CreateGroupMessage_H
#define	_INCLUDED_CreateGroupMessage_H

// ======================================================================

#include "sharedNetworkMessages/GameNetworkMessage.h"
#include "serverNetworkMessages/GroupMemberParam.h"
#include "serverNetworkMessages/GroupMemberParamArchive.h"

// ======================================================================

class CreateGroupMessage: public GameNetworkMessage
{
public:
	CreateGroupMessage(GroupMemberParam const &leader, stdvector<GroupMemberParam>::fwd const & members);
	CreateGroupMessage(Archive::ReadIterator &source);
	~CreateGroupMessage();

	GroupMemberParam const & getLeader() const;
	stdvector<GroupMemberParam>::fwd const & getMembers() const;

private:
	CreateGroupMessage(CreateGroupMessage const &);
	CreateGroupMessage &operator=(CreateGroupMessage const &);

	Archive::AutoVariable<GroupMemberParam> m_leader;
	Archive::AutoVariable<stdvector<GroupMemberParam>::fwd > m_members;
};

// ----------------------------------------------------------------------

inline GroupMemberParam const &CreateGroupMessage::getLeader() const
{
	return m_leader.get();
}

// ----------------------------------------------------------------------

inline stdvector<GroupMemberParam>::fwd const &CreateGroupMessage::getMembers() const
{
	return m_members.get();
}

// ======================================================================

#endif	// _INCLUDED_CreateGroupMessage_H

