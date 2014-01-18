// ConfigTransferServer.cpp
// Copyright 2000-01, Sony Online Entertainment Inc., all rights reserved.
// Author: Justin Randall

//-----------------------------------------------------------------------

#include "FirstTransferServer.h"
#include "sharedFoundation/ConfigFile.h"
#include "sharedNetwork/SetupSharedNetwork.h"
#include "ConfigTransferServer.h"

#include <vector>

//-----------------------------------------------------------------------

ConfigTransferServer::Data *	ConfigTransferServer::data = 0;

#define KEY_INT(a,b)    (data->a = ConfigFile::getKeyInt("TransferServer", #a, b))
#define KEY_BOOL(a,b)   (data->a = ConfigFile::getKeyBool("TransferServer", #a, b))
#define KEY_STRING(a,b) (data->a = ConfigFile::getKeyString("TransferServer", #a, b))

//-----------------------------------------------------------------------

namespace ConfigTransferServerNamespace
{
	std::vector<std::string> s_serversAllowedToUploadCharacterData;
	std::vector<std::string> s_serversAllowedToDownloadCharacterData;
}

using namespace ConfigTransferServerNamespace;

// ----------------------------------------------------------------------

const std::vector<std::string> & ConfigTransferServer::getServersAllowedToUploadCharacterData()
{
	return s_serversAllowedToUploadCharacterData;
}

// ----------------------------------------------------------------------

const std::vector<std::string> & ConfigTransferServer::getServersAllowedToDownloadCharacterData()
{
	return s_serversAllowedToDownloadCharacterData;
}

// ----------------------------------------------------------------------

void ConfigTransferServer::install(void)
{
	SetupSharedNetwork::SetupData  networkSetupData;
	SetupSharedNetwork::getDefaultServerSetupData(networkSetupData);

	data = new ConfigTransferServer::Data;

	KEY_INT     (centralServerServiceBindPort, 50005);
	KEY_STRING  (centralServerServiceBindInterface, "");
	KEY_BOOL    (allowSameServerTransfers, false);
	KEY_BOOL    (allowAccountTransfers, true);
	KEY_STRING  (apiServerHostAddress, "");
	KEY_BOOL    (transferChatAvatar, false);
	
	int index = 0;
	char const * result = 0;
	do
	{
		result = ConfigFile::getKeyString("TransferServer", "serverAllowedToUploadCharacterData", index++, 0);
		if (result != 0)
		{
			s_serversAllowedToUploadCharacterData.push_back(result);
		}
	}
	while (result);

	index = 0;
	result = 0;
	do
	{
		result = ConfigFile::getKeyString("TransferServer", "serverAllowedToDownloadCharacterData", index++, 0);
		if (result != 0)
		{
			s_serversAllowedToDownloadCharacterData.push_back(result);
		}
	}
	while (result);
}

//-----------------------------------------------------------------------

void ConfigTransferServer::remove(void)
{
	delete data;
	data = 0;
}

//-----------------------------------------------------------------------

unsigned short ConfigTransferServer::getCentralServerServiceBindPort()
{
	return static_cast<unsigned short>(data->centralServerServiceBindPort);
}

//-----------------------------------------------------------------------

const char * ConfigTransferServer::getCentralServerServiceBindInterface()
{
	return data->centralServerServiceBindInterface;
}

//-----------------------------------------------------------------------

bool ConfigTransferServer::getAllowSameServerTransfers()
{
	return data->allowSameServerTransfers;
}

//-----------------------------------------------------------------------

bool ConfigTransferServer::getAllowAccountTransfers()
{
	return data->allowAccountTransfers;
}

//-----------------------------------------------------------------------

const char * ConfigTransferServer::getApiServerHostAddress()
{
	return data->apiServerHostAddress;
}

//-----------------------------------------------------------------------

const bool ConfigTransferServer::getTransferChatAvatar()
{
	return data->transferChatAvatar;
}

//-----------------------------------------------------------------------
