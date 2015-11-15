// SystemAssignedProcessId.h
// Copyright 2000-01, Sony Online Entertainment Inc., all rights reserved. 
// Author: Justin Randall

#ifndef	_INCLUDED_SystemAssignedProcessId_H
#define	_INCLUDED_SystemAssignedProcessId_H

//-----------------------------------------------------------------------

#include "sharedNetworkMessages/GameNetworkMessage.h"

//-----------------------------------------------------------------------

class SystemAssignedProcessId : public GameNetworkMessage
{
public:
	explicit SystemAssignedProcessId(const unsigned long id);
	explicit SystemAssignedProcessId(Archive::ReadIterator & source);
	~SystemAssignedProcessId();

	const unsigned long getId  () const;

private:
	SystemAssignedProcessId & operator = (const SystemAssignedProcessId & rhs);
	SystemAssignedProcessId(const SystemAssignedProcessId & source);

	Archive::AutoVariable<unsigned long>  m_id;
};

//-----------------------------------------------------------------------

#endif	// _INCLUDED_SystemAssignedProcessId_H
