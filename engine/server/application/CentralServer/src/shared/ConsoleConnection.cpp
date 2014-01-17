// ConsoleConnection.cpp
// Copyright 2000-02, Sony Online Entertainment Inc., all rights reserved. 
// Author: Justin Randall

//-----------------------------------------------------------------------

#include "FirstCentralServer.h"
#include "ConsoleConnection.h"
#include "CentralServer.h"
#include "ConsoleCommandParser.h"
#include "ConsoleManager.h"
#include "sharedNetworkMessages/ConsoleChannelMessages.h"
#include "UnicodeUtils.h"

//-----------------------------------------------------------------------

namespace ConsoleConnectionNamespace
{
	std::map<int, ConsoleConnection *>  s_resultsMap;
	int                                 s_track = 0;
}

using namespace ConsoleConnectionNamespace;

//-----------------------------------------------------------------------

ConsoleConnection::ConsoleConnection(UdpConnectionMT * u, TcpClient * t) :
ServerConnection(u, t)
{
}

//-----------------------------------------------------------------------

ConsoleConnection::~ConsoleConnection()
{
	std::map<int, ConsoleConnection *>::iterator i = s_resultsMap.begin();

	while(i != s_resultsMap.end())
	{
		if((*i).second == this)
			s_resultsMap.erase(i++);
		else
			++i;
	}
}

//-----------------------------------------------------------------------

void ConsoleConnection::parseCommand(const std::string & cmd)
{
	int i = s_track;
	s_resultsMap[i] = this;
	++s_track;
	std::string result;
	CommandParser::ErrorType err = ConsoleManager::processString(cmd, i, result);
	if(err == CommandParser::ERR_CMD_NOT_FOUND)
	{
		onCommandComplete(result, i);
	}
	else if(!result.empty() && err == CommandParser::ERR_SUCCESS)
	{
		onCommandComplete(result, i);
	}
}

//-----------------------------------------------------------------------

void ConsoleConnection::onCommandComplete(const std::string & result, const int trackId)
{
	std::map<int, ConsoleConnection *>::iterator f = s_resultsMap.find(trackId);
	if(f != s_resultsMap.end())
	{
		ConsoleConnection * c = (*f).second;
		ConGenericMessage msg(result);
		c->send(msg, true);
		GameNetworkMessage dis("RequestDisconnect");
		c->send(dis, true);
	}
}

//-----------------------------------------------------------------------

void ConsoleConnection::onReceive(const Archive::ByteStream & bs)
{
	Archive::ReadIterator ri = bs.begin();
	GameNetworkMessage message(ri);
	ri = bs.begin();

	if(message.isType("ConGenericMessage"))
	{
		ConGenericMessage con(ri);
		parseCommand(con.getMsg());
	}
}

//-----------------------------------------------------------------------

