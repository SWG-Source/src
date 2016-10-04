// KeyServer.cpp
// copyright 2000 Verant Interactive
// Author: Justin Randall


//-----------------------------------------------------------------------

#include "serverKeyShare/FirstServerKeyShare.h"
#include "sharedFoundation/Clock.h"
#include "serverKeyShare/KeyServer.h"
#include "sharedRandom/RandomGenerator.h"
#include <time.h>

//-----------------------------------------------------------------------

KeyServer::KeyServer(const unsigned int newKeyCount, const real newUpdateRate) :
KeyShare(newKeyCount),
lastUpdateTime(CONST_REAL(0.0)),
randomNumberGenerator(0),
updateRate(newUpdateRate)
{
	randomNumberGenerator = new RandomGenerator(unsigned(time(0))); //lint !e732 //loss of sign ok.
	Key	k;
	for(unsigned int i = 0; i < getKeyCount(); i ++)
	{
		for(unsigned int j = 0; j < KeyShareConstants::keyLength; j ++)
		{
			k.value[j] = static_cast<unsigned char>(randomNumberGenerator->random(255));
		}
		setKey(k, i);
	}
}

//-----------------------------------------------------------------------

KeyServer::~KeyServer()
{
	delete randomNumberGenerator;
}

//-----------------------------------------------------------------------

bool KeyServer::update(void)
{
	return false;
}

//-----------------------------------------------------------------------

