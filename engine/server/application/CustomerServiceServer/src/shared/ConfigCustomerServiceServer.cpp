// ConfigCustomerServiceServer.cpp
// copyright 2000 Verant Interactive

//-----------------------------------------------------------------------

#include "FirstCustomerServiceServer.h"
#include "sharedFoundation/ConfigFile.h"
#include "sharedNetwork/SetupSharedNetwork.h"
#include "ConfigCustomerServiceServer.h"

//-----------------------------------------------------------------------

namespace ConfigCustomerServiceServerNamespace
{
	const char * s_clusterName = nullptr;
	const char * s_centralServerAddress = nullptr;
	int          s_centralServerPort = 0;
	const char * s_gameCode = nullptr;
	const char * s_csServerAddress = nullptr;
	int          s_csServerPort = 0;
	int          s_maxPacketsPerSecond = 50;
	int          s_requestTimeoutSeconds = 300;
	int          s_maxAllowedNumberOfTickets = 1;
	int          s_gameServicePort = 0;
	int          s_chatServicePort = 0;
	const char* s_chatServiceBindInterface = nullptr;
	const char* s_gameServiceBindInterface = nullptr;
	bool         s_writeTicketToBugLog = false;
};

using namespace ConfigCustomerServiceServerNamespace;

//-----------------------------------------------------------------------

#define KEY_INT(a,b)    (s_ ## a = ConfigFile::getKeyInt("CustomerServiceServer", #a, b))
#define KEY_STRING(a,b) (s_ ## a = ConfigFile::getKeyString("CustomerServiceServer", #a, b))
#define KEY_BOOL(a,b)   (s_ ## a = ConfigFile::getKeyBool("CustomerServiceServer", #a, b))

//-----------------------------------------------------------------------

void ConfigCustomerServiceServer::install()
{
	SetupSharedNetwork::SetupData  networkSetupData;
	SetupSharedNetwork::getDefaultServerSetupData(networkSetupData);
	SetupSharedNetwork::install(networkSetupData);

	KEY_STRING (clusterName, "devcluster");
	KEY_STRING (centralServerAddress, "localhost");
	KEY_INT    (centralServerPort, 61242);
	KEY_STRING (gameCode, "SWG");
	KEY_STRING (csServerAddress, "localhost");
	KEY_INT    (csServerPort, 3016);
	KEY_INT    (maxPacketsPerSecond, 50);
	KEY_INT    (requestTimeoutSeconds, 300);
	KEY_INT    (maxAllowedNumberOfTickets, 1);
	KEY_INT    (gameServicePort, 50010);
	KEY_STRING (gameServiceBindInterface, "");
	KEY_INT    (chatServicePort, 50011);
	KEY_STRING (chatServiceBindInterface, "");
	KEY_BOOL   (writeTicketToBugLog, false);
}

//-----------------------------------------------------------------------

void ConfigCustomerServiceServer::remove()
{
}

//-----------------------------------------------------------------------

int ConfigCustomerServiceServer::getMaxPacketsPerSecond()
{
	return s_maxPacketsPerSecond;
}

//-----------------------------------------------------------------------

unsigned int ConfigCustomerServiceServer::getRequestTimeoutSeconds()
{
	return static_cast<unsigned int>(s_requestTimeoutSeconds);
}

//-----------------------------------------------------------------------

const char * ConfigCustomerServiceServer::getCustomerServiceServerAddress() 
{
	return s_csServerAddress;
}

//-----------------------------------------------------------------------

unsigned short ConfigCustomerServiceServer::getCustomerServiceServerPort()
{
	return static_cast<unsigned short>(s_csServerPort);
}

//-----------------------------------------------------------------------

const char * ConfigCustomerServiceServer::getCentralServerAddress() 
{
	return s_centralServerAddress;
}

//-----------------------------------------------------------------------

unsigned short ConfigCustomerServiceServer::getCentralServerPort()
{
	return static_cast<unsigned short>(s_centralServerPort);
}

//-----------------------------------------------------------------------

const char * ConfigCustomerServiceServer::getClusterName()
{
	return s_clusterName;
}

//-----------------------------------------------------------------------

const char * ConfigCustomerServiceServer::getGameCode() 
{
	return s_gameCode;
}

//-----------------------------------------------------------------------

int ConfigCustomerServiceServer::getMaxAllowedNumberOfTickets()
{
	return s_maxAllowedNumberOfTickets;
}

//-----------------------------------------------------------------------

unsigned short ConfigCustomerServiceServer::getGameServicePort()
{
	return static_cast<unsigned short>(s_gameServicePort);
}

//-----------------------------------------------------------------------

unsigned short ConfigCustomerServiceServer::getChatServicePort()
{
	return static_cast<unsigned short>(s_chatServicePort);
}

//-----------------------------------------------------------------------

const char* ConfigCustomerServiceServer::getGameServiceBindInterface()
{
	return s_gameServiceBindInterface;
}

//-----------------------------------------------------------------------

const char* ConfigCustomerServiceServer::getChatServiceBindInterface()
{
	return s_chatServiceBindInterface;
}

//-----------------------------------------------------------------------

bool ConfigCustomerServiceServer::getWriteTicketToBugLog()
{
	return s_writeTicketToBugLog;
}

//-----------------------------------------------------------------------
