// ClientLoginMessages.cpp
// copyright 2001 Verant Interactive
// Author: Justin Randall

//-----------------------------------------------------------------------

#include "sharedNetworkMessages/FirstSharedNetworkMessages.h"
#include "sharedNetworkMessages/ClientLoginMessages.h"

#include "sharedFoundation/ConfigFile.h"

//-----------------------------------------------------------------------

//-- todo add SetupSharedNetworkMessages.* and ConfigSharedNetworkMessages.*
#define ConfigVersion ConfigFile::getKeyString("SharedNetworkMessages", "version", 0)

LoginClientId::LoginClientId(const std::string & newId, const std::string & newKey) :
GameNetworkMessage("LoginClientId"),
id(newId),
key(newKey),
version(ConfigVersion ? ConfigVersion : NetworkVersionId)  //-- todo production build should not allow this
{
	WARNING(ConfigVersion, ("Network version override detected: %s", ConfigVersion));
	addVariable(id);
	addVariable(key);
	addVariable(version);
}

#undef ConfigVersion

//-----------------------------------------------------------------------

LoginClientId::LoginClientId(Archive::ReadIterator & source) :
GameNetworkMessage("LoginClientId"),
id(),
key(),
version()
{
	addVariable(id);
	addVariable(key);
	addVariable(version);
	unpack(source);
}

//-----------------------------------------------------------------------

LoginClientId::~LoginClientId()
{
}

//-----------------------------------------------------------------------

LoginClientToken::LoginClientToken(const unsigned char * const newToken, const unsigned char newTokenSize, uint32 suid, const std::string & username) :
GameNetworkMessage("LoginClientToken"),
token      (),
tokenData  (0),
stationId  (suid),
m_username (username)
{

	tokenData = new unsigned char[newTokenSize];
	memcpy(tokenData, newToken, newTokenSize);

	// todo: preallocated token with newTokenSize bytes
	std::vector<unsigned char> a;
	for(unsigned int i = 0; i < newTokenSize; ++i)
	{
		a.push_back(newToken[i]);
	}
	token.set(a);
	addVariable(token);
	addVariable(stationId);
	addVariable(m_username);
}

//-----------------------------------------------------------------------

LoginClientToken::LoginClientToken(Archive::ReadIterator & source) :
GameNetworkMessage("LoginClientToken"),
token      (),
tokenData  (0),
stationId  (0),
m_username ()
{
	addVariable(token);
	addVariable(stationId);
	addVariable(m_username);
	AutoByteStream::unpack(source);

	unsigned int tokenSize = token.get().size();
	tokenData = new unsigned char[tokenSize];
	for(unsigned int i = 0; i < tokenSize; i ++)
	{
		tokenData[i] = token.get()[i];
	}
}

//-----------------------------------------------------------------------

LoginClientToken::~LoginClientToken()
{
	delete[] tokenData;
}

//-----------------------------------------------------------------------

LoginIncorrectClientId::LoginIncorrectClientId(const std::string & newServerId, const std::string & newServerApplicationVersion) :
GameNetworkMessage("LoginIncorrectClientId"),
serverId(newServerId),
serverApplicationVersion(newServerApplicationVersion)
{
	addVariable(serverId);
	addVariable(serverApplicationVersion);
}

//-----------------------------------------------------------------------

LoginIncorrectClientId::LoginIncorrectClientId(Archive::ReadIterator & source) :
GameNetworkMessage("LoginIncorrectClientId"),
serverId(),
serverApplicationVersion()
{
	addVariable(serverId);
	addVariable(serverApplicationVersion);
	unpack(source);
}

//-----------------------------------------------------------------------

LoginIncorrectClientId::~LoginIncorrectClientId()
{
}

//-----------------------------------------------------------------------

