// ClientLoginMessages.h
// copyright 2001 Verant Interactive
// Author: Justin Randall

#ifndef	_INCLUDED_ClientLoginMessages_H
#define	_INCLUDED_ClientLoginMessages_H

//-----------------------------------------------------------------------

#include "sharedNetworkMessages/GameNetworkMessage.h"

namespace ClientLoginMessageConstants
{
	const unsigned char        EnumerateCluster = 1;
	const unsigned char        ClientLoginId    = 2;
	const unsigned char        LoginToken       = 3;
}

//-----------------------------------------------------------------------

class LoginClientId : public GameNetworkMessage
{
public:
	LoginClientId(const std::string & newId, const std::string & newKey);
	LoginClientId(Archive::ReadIterator & source);
	~LoginClientId();
	const std::string &  getId  () const;
	const std::string &  getKey () const;
	const std::string &  getVersion () const;
private:
	Archive::AutoVariable<std::string>          id;
	Archive::AutoVariable<std::string>          key;
	Archive::AutoVariable<std::string>          version;

	LoginClientId();
	LoginClientId(const LoginClientId&);
	LoginClientId& operator= (const LoginClientId&);
};

//-----------------------------------------------------------------------

inline const std::string & LoginClientId::getId() const
{
	return id.get();
}

//-----------------------------------------------------------------------

inline const std::string & LoginClientId::getKey() const
{
	return key.get();
}

//-----------------------------------------------------------------------

inline const std::string & LoginClientId::getVersion() const
{
	return version.get();
}

//-----------------------------------------------------------------------

class LoginClientToken : public GameNetworkMessage
{
public:
	LoginClientToken(const unsigned char * const newToken, const unsigned char newTokenSize, uint32 suid, const std::string & username);
	LoginClientToken(Archive::ReadIterator & source);
	~LoginClientToken();

	const unsigned char * const getToken     () const;
	const unsigned char         getTokenSize () const;
	const uint32                getStationId () const;
	const std::string &         getUsername  () const;

	
private:
	Archive::AutoArray<unsigned char>  token;
	unsigned char *                    tokenData;
	Archive::AutoVariable<uint32>      stationId;
	Archive::AutoVariable<std::string> m_username;

	LoginClientToken();
	LoginClientToken(const LoginClientToken&);
	LoginClientToken& operator= (const LoginClientToken&);
};

//-----------------------------------------------------------------------

inline const unsigned char * const LoginClientToken::getToken() const
{
	return tokenData;
}

//-----------------------------------------------------------------------

inline const unsigned char LoginClientToken::getTokenSize() const
{
	return static_cast<unsigned char>(token.get().size());
}

//-----------------------------------------------------------------------

inline const uint32 LoginClientToken::getStationId() const
{
	return stationId.get();
}

//-----------------------------------------------------------------------

inline const std::string & LoginClientToken::getUsername  () const
{
	return m_username.get ();
}

//-----------------------------------------------------------------------

class LoginIncorrectClientId : public GameNetworkMessage
{
public:
	LoginIncorrectClientId(const std::string & newServerId, const std::string & newServerApplicationVersion);
	LoginIncorrectClientId(Archive::ReadIterator & source);
	~LoginIncorrectClientId();
	const std::string &  getServerId                  () const;
	const std::string &  getServerApplicationVersion  () const;
private:
	Archive::AutoVariable<std::string>          serverId;
	Archive::AutoVariable<std::string>          serverApplicationVersion;

	LoginIncorrectClientId();
	LoginIncorrectClientId(const LoginIncorrectClientId&);
	LoginIncorrectClientId& operator= (const LoginIncorrectClientId&);
};

//-----------------------------------------------------------------------

inline const std::string & LoginIncorrectClientId::getServerId() const
{
	return serverId.get();
}

//-----------------------------------------------------------------------

inline const std::string & LoginIncorrectClientId::getServerApplicationVersion() const
{
	return serverApplicationVersion.get();
}

//-----------------------------------------------------------------------

#endif	// _INCLUDED_ClientLoginMessages_H
