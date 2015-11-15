// TaskSpawnProcessAck.cpp
// Copyright 2000-04, Sony Online Entertainment Inc., all rights reserved. 
// Author: Justin Randall

//-----------------------------------------------------------------------

#include "serverNetworkMessages/FirstServerNetworkMessages.h"
#include "TaskSpawnProcessAck.h"

//-----------------------------------------------------------------------

TaskSpawnProcessAck::TaskSpawnProcessAck(const int transactionId) :
GameNetworkMessage("TaskSpawnProcessAck"),
m_transactionId(transactionId)
{
	addVariable(m_transactionId);
}

//-----------------------------------------------------------------------

TaskSpawnProcessAck::TaskSpawnProcessAck(Archive::ReadIterator & source) :
GameNetworkMessage("TaskSpawnProcessAck"),
m_transactionId()
{
	addVariable(m_transactionId);
	unpack(source);
}

//-----------------------------------------------------------------------

TaskSpawnProcessAck::~TaskSpawnProcessAck()
{
}

//-----------------------------------------------------------------------

int TaskSpawnProcessAck::getTransactionId() const
{
	return m_transactionId.get();
}

//-----------------------------------------------------------------------

