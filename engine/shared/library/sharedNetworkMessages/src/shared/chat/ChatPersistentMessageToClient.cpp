// ChatPersistentMessageToClient.cpp
// Copyright 2000-02, Sony Online Entertainment Inc., all rights reserved. 
// Author: Justin Randall

//-----------------------------------------------------------------------

#include "sharedNetworkMessages/FirstSharedNetworkMessages.h"
#include "sharedNetworkMessages/ChatPersistentMessageToClient.h"
#include "sharedNetworkMessages/ChatPersistentMessageToClientDataArchive.h"

//-----------------------------------------------------------------------

ChatPersistentMessageToClient::ChatPersistentMessageToClient(const unsigned int i, signed char stat, const std::string & g, const std::string & s, const std::string & c, const Unicode::String & subj, const unsigned int t) :
GameNetworkMessage("ChatPersistentMessageToClient"),
data ()
{
	Data d;

	d.fromCharacterName = c;
	d.fromGameCode = g;
	d.fromServerCode = s;
	d.id = i;
	d.isHeader = true;
	d.subject = subj;
	d.status = stat;
	d.timeStamp = t;

	data.set (d);

	addVariable (data);
}

//-----------------------------------------------------------------------

ChatPersistentMessageToClient::ChatPersistentMessageToClient(const unsigned int i, signed char stat, const std::string & g, const std::string & s, const std::string & c, const Unicode::String & subj, const Unicode::String & m, const Unicode::String & o, const unsigned int t) :
GameNetworkMessage("ChatPersistentMessageToClient"),
data ()
{
	Data d;

	d.fromCharacterName = c;
	d.fromGameCode = g;
	d.fromServerCode = s;
	d.id = i;
	d.isHeader = false;
	d.subject = subj;
	d.message = m;
	d.outOfBand = o;
	d.status = stat;
	d.timeStamp = t;

	data.set (d);

	addVariable (data);
}

//-----------------------------------------------------------------------

ChatPersistentMessageToClient::ChatPersistentMessageToClient(Archive::ReadIterator & source) :
GameNetworkMessage("ChatPersistentMessageToClient"),
data ()
{
	addVariable (data);
	unpack(source);
}

//-----------------------------------------------------------------------

ChatPersistentMessageToClient::~ChatPersistentMessageToClient()
{
}

//-----------------------------------------------------------------------

ChatPersistentMessageToClient & ChatPersistentMessageToClient::operator = (const ChatPersistentMessageToClient & rhs)
{
	if(this != &rhs)
	{
		*this = rhs;
	}
	return *this;
}

//-----------------------------------------------------------------------

