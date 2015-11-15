///////////////////////////////////////////////////////////////////////////////
//
// ChatOnDeleteAllPersistentMessages.cpp
// Copyright 2003, Sony Online Entertainment Inc., all rights reserved. 
//
///////////////////////////////////////////////////////////////////////////////

#include "sharedNetworkMessages/FirstSharedNetworkMessages.h"
#include "ChatOnDeleteAllPersistentMessages.h"

//-----------------------------------------------------------------------------

ChatOnDeleteAllPersistentMessages::ChatOnDeleteAllPersistentMessages(std::string const &targetName, bool const success)
 : GameNetworkMessage("ChatOnDeleteAllPersistentMessages")
 , m_targetName(targetName)
 , m_success(success)
{
	addVariable(m_targetName);
	addVariable(m_success);
}

//-----------------------------------------------------------------------------
ChatOnDeleteAllPersistentMessages::ChatOnDeleteAllPersistentMessages(Archive::ReadIterator & source)
 : GameNetworkMessage("ChatOnDeleteAllPersistentMessages")
 , m_targetName()
 , m_success()
{
	addVariable(m_targetName);
	addVariable(m_success);
	unpack(source);
}

//-----------------------------------------------------------------------------

ChatOnDeleteAllPersistentMessages::~ChatOnDeleteAllPersistentMessages()
{
}

//-----------------------------------------------------------------------------

std::string const &ChatOnDeleteAllPersistentMessages::getTargetName() const
{
	return m_targetName.get();
}

//-----------------------------------------------------------------------------

bool const &ChatOnDeleteAllPersistentMessages::isSuccess() const
{
	return m_success.get();
}

///////////////////////////////////////////////////////////////////////////////
