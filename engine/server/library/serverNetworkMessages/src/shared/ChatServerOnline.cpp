// ChatServerOnline.cpp
// Copyright 2000-02, Sony Online Entertainment Inc., all rights reserved. 
// Author: Justin Randall

//-----------------------------------------------------------------------

#include "serverNetworkMessages/FirstServerNetworkMessages.h"
#include "serverNetworkMessages/ChatServerOnline.h"

//-----------------------------------------------------------------------

ChatServerOnline::ChatServerOnline(const std::string & a, const unsigned short p) :
GameNetworkMessage("ChatServerOnline"),
address(a),
port(p)
{
	addVariable(address);
	addVariable(port);
}

//-----------------------------------------------------------------------

ChatServerOnline::ChatServerOnline(Archive::ReadIterator & source) :
GameNetworkMessage("ChatServerOnline"),
address(),
port()
{
	addVariable(address);
	addVariable(port);
	unpack(source);
}

//-----------------------------------------------------------------------

ChatServerOnline::~ChatServerOnline()
{
}

//-----------------------------------------------------------------------

