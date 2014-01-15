// ConnectionServerDown.cpp
// Copyright 2000-02, Sony Online Entertainment Inc., all rights reserved. 
// Author: Justin Randall

//-----------------------------------------------------------------------

#include "serverNetworkMessages/FirstServerNetworkMessages.h"
#include "serverNetworkMessages/ConnectionServerDown.h"

//-----------------------------------------------------------------------

ConnectionServerDown::ConnectionServerDown(int id) :
GameNetworkMessage("ConnectionServerDown"),
m_id(id)
{
	addVariable(m_id);
}

//-----------------------------------------------------------------------

ConnectionServerDown::ConnectionServerDown(Archive::ReadIterator & source) :
GameNetworkMessage("ConnectionServerDown"),
m_id(0)
{
	addVariable(m_id);
	AutoByteStream::unpack(source);
}

//-----------------------------------------------------------------------

ConnectionServerDown::~ConnectionServerDown()
{
}

//-----------------------------------------------------------------------

