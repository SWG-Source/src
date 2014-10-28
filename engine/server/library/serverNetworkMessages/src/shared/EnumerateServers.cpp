// EnumerateServers.cpp
// Copyright 2000-02, Sony Online Entertainment Inc., all rights reserved. 
// Author: Justin Randall

//-----------------------------------------------------------------------

#include "serverNetworkMessages/FirstServerNetworkMessages.h"
#include "EnumerateServers.h"

//-----------------------------------------------------------------------

EnumerateServers::EnumerateServers(const bool a, const std::string &addr, const unsigned short p, const int t) :
GameNetworkMessage("EnumerateServers"),
add(a),
address(addr),
port(p),
serverType(t)
{
	addVariable(add);
	addVariable(address);
	addVariable(port);
	addVariable(serverType);
}

//-----------------------------------------------------------------------

EnumerateServers::EnumerateServers(Archive::ReadIterator & source) :
GameNetworkMessage("EnumerateServers"),
add(),
address(),
port(),
serverType()
{
	addVariable(add);
	addVariable(address);
	addVariable(port);
	addVariable(serverType);
	unpack(source);
}

//-----------------------------------------------------------------------

EnumerateServers::~EnumerateServers()
{
}

//-----------------------------------------------------------------------

