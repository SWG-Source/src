// SystemAssignedProcessId.cpp
// Copyright 2000-01, Sony Online Entertainment Inc., all rights reserved. 
// Author: Justin Randall

//-----------------------------------------------------------------------

#include "serverUtility/FirstServerUtility.h"
#include "SystemAssignedProcessId.h"

//-----------------------------------------------------------------------

SystemAssignedProcessId::SystemAssignedProcessId(const unsigned long id) :
GameNetworkMessage("SystemAssignedProcessId"),
m_id(id)
{
	addVariable(m_id);
}

//-----------------------------------------------------------------------

SystemAssignedProcessId::SystemAssignedProcessId(Archive::ReadIterator & source) :
GameNetworkMessage("SystemAssignedProcessId"),
m_id()
{
	addVariable(m_id);
	unpack(source);
}

//-----------------------------------------------------------------------

SystemAssignedProcessId::~SystemAssignedProcessId()
{
}

//-----------------------------------------------------------------------

const unsigned long SystemAssignedProcessId::getId() const
{
	return m_id.get();
}

//-----------------------------------------------------------------------

