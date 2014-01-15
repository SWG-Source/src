// LoginConnectionServerAddress.h
// Copyright 2000-02, Sony Online Entertainment Inc., all rights reserved. 
// Author: Justin Randall

#ifndef	_INCLUDED_LoginConnectionServerAddress_H
#define	_INCLUDED_LoginConnectionServerAddress_H

//-----------------------------------------------------------------------

#include "sharedNetworkMessages/GameNetworkMessage.h"

//-----------------------------------------------------------------------

class LoginConnectionServerAddress : public GameNetworkMessage
{
public:

	LoginConnectionServerAddress (int id, const std::string & newServiceAddress, const uint16 newClientPortPrivate, const uint16 newClientPortPublic, int newNumClients, const uint16 _pingPort);
	LoginConnectionServerAddress (Archive::ReadIterator & source);
    virtual ~LoginConnectionServerAddress ();

    const std::string &            getClientServiceAddress() const;
    uint16                         getClientServicePortPrivate() const;
    uint16                         getClientServicePortPublic() const;
	int                            getId() const;
	int                            getNumClients() const;
    uint16                         getPingPort () const;

private:
	Archive::AutoVariable<std::string>   clientServiceAddress;
	Archive::AutoVariable<uint16>        clientServicePortPrivate;
	Archive::AutoVariable<uint16>        clientServicePortPublic;
	Archive::AutoVariable<int>           id;
	Archive::AutoVariable<int>           numClients;
	Archive::AutoVariable<uint16>        pingPort;

	LoginConnectionServerAddress(const LoginConnectionServerAddress&);
	LoginConnectionServerAddress& operator= (const LoginConnectionServerAddress&);
};
//-----------------------------------------------------------------------

inline int LoginConnectionServerAddress::getId() const
{
	return id.get();
}


//-----------------------------------------------------------------------

inline const std::string & LoginConnectionServerAddress::getClientServiceAddress (void) const
{
	return clientServiceAddress.get();
}

//-----------------------------------------------------------------------

inline uint16 LoginConnectionServerAddress::getClientServicePortPrivate (void) const
{
	return clientServicePortPrivate.get();
}

//-----------------------------------------------------------------------

inline uint16 LoginConnectionServerAddress::getClientServicePortPublic (void) const
{
	return clientServicePortPublic.get();
}

//----------------------------------------------------------------------
inline int LoginConnectionServerAddress::getNumClients (void) const
{
	return numClients.get();
}

//----------------------------------------------------------------------

inline uint16 LoginConnectionServerAddress::getPingPort () const
{
	return pingPort.get ();
}

//-----------------------------------------------------------------------

#endif	
