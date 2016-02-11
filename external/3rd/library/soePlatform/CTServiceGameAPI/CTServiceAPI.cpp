#pragma warning (disable : 4786)
//test
#include <string.h>
#include "CTServiceAPI.h"
#include "CTServiceAPICore.h"
#include "Request.h"
#include "Response.h"

const unsigned short EMPTY_STRING[1] = { 0 };
const char * DEFAULT_GAMECODE = "SWG";
const char * DEFAULT_HOST = "localhost";
const unsigned short DEFAULT_PORT = 2000;

namespace CTService 
{

using namespace Base;

//-----------------------------------------------
CTServiceAPI::CTServiceAPI(const char *hostName, const char *game)
//-----------------------------------------------
{
	std::vector<const char *> hostArray;
	std::vector<short> portArray;
	char hostConfig[4096];
	if (hostName == nullptr) 
		hostName = DEFAULT_HOST;
	if (!game)
		game = DEFAULT_GAMECODE;
	strncpy(hostConfig, hostName, 4096); hostConfig[4095] = 0;
	if (char * ptr = strtok(hostConfig, " "))
	{
		do
		{
			char * host = ptr;
			char * portStr = strchr(host, ':');
			unsigned short port = DEFAULT_PORT;
			if (portStr)
			{
				*portStr++ = 0;
				port = atoi(portStr);
			}

			if (::strlen(host) && port)
			{
				hostArray.push_back(host);
				portArray.push_back(port);
			}
		}
		while ((ptr = strtok(nullptr, " ")) != nullptr);
	}
	if (hostArray.empty())
	{
		hostArray.push_back(DEFAULT_HOST);
		portArray.push_back(DEFAULT_PORT);
	}
	m_apiCore = new CTServiceAPICore(&hostArray[0], &portArray[0], hostArray.size(), this, game);
}

//-----------------------------------------------
CTServiceAPI::~CTServiceAPI()
//-----------------------------------------------
{
    delete m_apiCore;
}


//-----------------------------------------------
void CTServiceAPI::process()
//-----------------------------------------------
{
	m_apiCore->process();
}

//-----------------------------------------------
//
// Add requests and replies here
//
//-----------------------------------------------
/*
//----------- TEST CODE ONLY ------------
//-----------------------------------------------
unsigned CTServiceAPI::requestTest(const char *astring, const unsigned anint, void *user)
//-----------------------------------------------
{
	std::string s_astring = astring;
	ReqTest *req = new ReqTest(s_astring, anint);
	return (m_apiCore->submitRequest(req, new ResTest(user)));
}

//-----------------------------------------------
unsigned CTServiceAPI::replyTest(const unsigned server_track, const unsigned value, void *user)
//-----------------------------------------------
{
	ReqReplyTest *req = new ReqReplyTest(server_track, value);
	return (m_apiCore->submitRequest(req, new ResReplyTest(user)));
}
//----------- TEST CODE ONLY ------------
*/

//-----------------------------------------------
unsigned CTServiceAPI::replyMoveStatus(const unsigned server_track, const unsigned status, const unsigned result, const CTUnicodeChar *reason, void *user)
//-----------------------------------------------
{
	if (!reason)
		reason = EMPTY_STRING;
	ReqReplyMoveStatus *req = new ReqReplyMoveStatus(server_track, status, result, reason);
	return (m_apiCore->submitRequest(req, new ResReplyMoveStatus(user)));
}

//-----------------------------------------------
unsigned CTServiceAPI::replyValidateMove(const unsigned server_track, const unsigned result, const CTUnicodeChar *reason, const CTUnicodeChar *suggestedName, void *user)
//-----------------------------------------------
{
	if (!reason)
		reason = EMPTY_STRING;
	if (!suggestedName)
		suggestedName = EMPTY_STRING;
	ReqReplyValidateMove *req = new ReqReplyValidateMove(server_track, result, reason, suggestedName);
	return (m_apiCore->submitRequest(req, new ResReplyValidateMove(user)));
}

//-----------------------------------------------
unsigned CTServiceAPI::replyMove(const unsigned server_track, const unsigned result, const CTUnicodeChar *reason, void *user)
//-----------------------------------------------
{
	if (!reason)
		reason = EMPTY_STRING;
	ReqReplyMove *req = new ReqReplyMove(server_track, result, reason);
	return (m_apiCore->submitRequest(req, new ResReplyMove(user)));
}

//-----------------------------------------------
unsigned CTServiceAPI::replyDelete(const unsigned server_track, const unsigned result, const CTUnicodeChar *reason, void *user)
//-----------------------------------------------
{
	if (!reason)
		reason = EMPTY_STRING;
	ReqReplyDelete *req = new ReqReplyDelete(server_track, result, reason);
	return (m_apiCore->submitRequest(req, new ResReplyDelete(user)));
}

//-----------------------------------------------
unsigned CTServiceAPI::replyRestore(const unsigned server_track, const unsigned result, const CTUnicodeChar *reason, void *user)
//-----------------------------------------------
{
	if (!reason)
		reason = EMPTY_STRING;
	ReqReplyRestore *req = new ReqReplyRestore(server_track, result, reason);
	return (m_apiCore->submitRequest(req, new ResReplyRestore(user)));
}

//-----------------------------------------------
unsigned CTServiceAPI::replyTransferAccount(const unsigned server_track, const unsigned result, const CTUnicodeChar *reason, void *user)
//-----------------------------------------------
{
	if (!reason)
		reason = EMPTY_STRING;
	ReqReplyTransferAccount *req = new ReqReplyTransferAccount(server_track, result, reason);
	return (m_apiCore->submitRequest(req, new ResReplyTransferAccount(user)));
}

//-----------------------------------------------
unsigned CTServiceAPI::replyCharacterList(const unsigned server_track, const unsigned result, const unsigned count, const CTServiceCharacter *characters, void *user)
//-----------------------------------------------
{
	ReqReplyCharacterList *req = new ReqReplyCharacterList(server_track, result, count, characters);
	return (m_apiCore->submitRequest(req, new ResReplyCharacterList(user)));
}

//-----------------------------------------------
unsigned CTServiceAPI::replyServerList(const unsigned server_track, const unsigned result, const unsigned count, const CTServiceServer *servers, void *user)
//-----------------------------------------------
{
	ReqReplyServerList *req = new ReqReplyServerList(server_track, result, count, servers);
	return (m_apiCore->submitRequest(req, new ResReplyServerList(user)));
}

//-----------------------------------------------
unsigned CTServiceAPI::replyDestinationServerList(const unsigned server_track, const unsigned result, const unsigned count, const CTServiceServer *servers, void *user)
//-----------------------------------------------
{
	ReqReplyDestinationServerList *req = new ReqReplyDestinationServerList(server_track, result, count, servers);
	return (m_apiCore->submitRequest(req, new ResReplyDestinationServerList(user)));
}

}; // namespace
