// LoginKeyPush.h
// Copyright 2000-02, Sony Online Entertainment Inc., all rights reserved. 
// Author: Justin Randall

#ifndef	_INCLUDED_LoginKeyPush_H
#define	_INCLUDED_LoginKeyPush_H

//-----------------------------------------------------------------------

#include "sharedNetworkMessages/GameNetworkMessage.h"
#include "serverKeyShare/KeyShare.h"

//-----------------------------------------------------------------------

class LoginKeyPush : public GameNetworkMessage
{
public:
	explicit LoginKeyPush	(const KeyShare::Key & sourceKey);
	LoginKeyPush            (Archive::ReadIterator & source);
	~LoginKeyPush	();
	const KeyShare::Key &		getKey (void) const;
private:
	AutoVariableKeyShare		key;

	LoginKeyPush();
	LoginKeyPush(const LoginKeyPush&);
	LoginKeyPush& operator= (const LoginKeyPush&);
};

//-----------------------------------------------------------------------

inline const KeyShare::Key & LoginKeyPush::getKey(void) const
{
	return key;
}

//-----------------------------------------------------------------------

#endif	// _INCLUDED_LoginKeyPush_H
