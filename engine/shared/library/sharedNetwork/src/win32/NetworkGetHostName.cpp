// NetworkGetHostName.cpp
// Copyright 2000-02, Sony Online Entertainment Inc., all rights reserved. 

//-----------------------------------------------------------------------

#include "sharedNetwork/FirstSharedNetwork.h"

#include "sharedNetwork/Address.h"
#include "sharedNetwork/NetworkHandler.h"

#include <winsock.h>

//-----------------------------------------------------------------------

struct HN
{
	HN();
	std::string hostName;
};

//-----------------------------------------------------------------------

HN::HN()
{
    WSADATA wsaData;
    int iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	char name[512] = {"\0"};
	if(gethostname(name, sizeof(name)) == 0)
	{
		Address a(name, 0);
		hostName = a.getHostAddress();//name;
	}
}

//-----------------------------------------------------------------------

const std::string & NetworkHandler::getHostName()
{
	static HN hn;
	return hn.hostName;
}

const std::string & NetworkHandler::getHumanReadableHostName()
{
	char name[512] = {"\0"};
	static std::string nameString;
	if(nameString.empty())
	{
		if(gethostname(name, sizeof(name)) == 0)
		{
			name[sizeof(name) - 1] = 0;
			//hostName = name;
			nameString = name;
		}
	}
	return nameString;
}

//-----------------------------------------------------------------------

const std::vector<std::pair<std::string, std::string> > & NetworkHandler::getInterfaceAddresses()
{
	static std::vector<std::pair<std::string, std::string> > s;
	return s;
}

//-----------------------------------------------------------------------
