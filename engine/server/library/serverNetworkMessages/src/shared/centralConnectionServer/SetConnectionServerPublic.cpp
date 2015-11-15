// SetConnectionServerPublic.cpp
// Copyright 2000-02, Sony Online Entertainment Inc., all rights reserved. 
// Author: Justin Randall

//-----------------------------------------------------------------------

#include "serverNetworkMessages/FirstServerNetworkMessages.h"
#include "serverNetworkMessages/SetConnectionServerPublic.h"

//-----------------------------------------------------------------------

SetConnectionServerPublic::SetConnectionServerPublic(const bool isPublic) :
GameNetworkMessage("SetConnectionServerPublic"),
m_isPublic(isPublic)
{
	addVariable(m_isPublic);
}

//-----------------------------------------------------------------------

SetConnectionServerPublic::SetConnectionServerPublic(Archive::ReadIterator & source) :
GameNetworkMessage("SetConnectionServerPublic"),
m_isPublic()
{
	addVariable(m_isPublic);
	unpack(source);
}

//-----------------------------------------------------------------------

SetConnectionServerPublic::~SetConnectionServerPublic()
{
}

//-----------------------------------------------------------------------

