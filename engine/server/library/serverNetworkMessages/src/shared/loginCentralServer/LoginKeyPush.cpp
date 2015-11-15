// LoginKeyPush.cpp
// Copyright 2000-02, Sony Online Entertainment Inc., all rights reserved. 
// Author: Justin Randall

//-----------------------------------------------------------------------

#include "serverNetworkMessages/FirstServerNetworkMessages.h"
#include "serverNetworkMessages/LoginKeyPush.h"

//-----------------------------------------------------------------------

LoginKeyPush::LoginKeyPush(const KeyShare::Key & sourceKey) :
GameNetworkMessage("LoginKeyPush"),
key(sourceKey)
{
    addVariable(key);
}

//-----------------------------------------------------------------------

LoginKeyPush::LoginKeyPush(Archive::ReadIterator & source) :
GameNetworkMessage("LoginKeyPush"),
key()
{
    addVariable(key);
    AutoByteStream::unpack(source);
}

//-----------------------------------------------------------------------

LoginKeyPush::~LoginKeyPush()
{
}


//-----------------------------------------------------------------------

