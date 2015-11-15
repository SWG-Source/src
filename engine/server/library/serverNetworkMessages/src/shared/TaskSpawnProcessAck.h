// TaskSpawnProcessAck.h
// Copyright 2000-04, Sony Online Entertainment Inc., all rights reserved. 
// Author: Justin Randall

#ifndef	_INCLUDED_TaskSpawnProcessAck_H
#define	_INCLUDED_TaskSpawnProcessAck_H

//-----------------------------------------------------------------------

#include "sharedNetworkMessages/GameNetworkMessage.h"

//-----------------------------------------------------------------------

class TaskSpawnProcessAck : public GameNetworkMessage
{
public:
	TaskSpawnProcessAck(int transactionId);
	TaskSpawnProcessAck(Archive::ReadIterator & r);
	~TaskSpawnProcessAck();

	int  getTransactionId  () const;

private:
	TaskSpawnProcessAck & operator = (const TaskSpawnProcessAck & rhs);
	TaskSpawnProcessAck(const TaskSpawnProcessAck & source);

	Archive::AutoVariable<int>            m_transactionId;
};

//-----------------------------------------------------------------------

#endif	// _INCLUDED_TaskSpawnProcessAck_H
