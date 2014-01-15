// KeyServer.h
// copyright 2000 Verant Interactive
// Author: Justin Randall

#ifndef	_KeyServer_H
#define	_KeyServer_H

//-----------------------------------------------------------------------

#include "serverKeyShare/KeyShare.h"

//-----------------------------------------------------------------------

class RandomGenerator;

//-----------------------------------------------------------------------

class KeyServer : public KeyShare
{
public:
	explicit KeyServer (const unsigned int newKeyCount=2, const real newUpdateRate = CONST_REAL(60000.0));
	~KeyServer ();
	bool update(void);

private:
	real				lastUpdateTime;
	RandomGenerator *	randomNumberGenerator;
	real				updateRate;

	KeyServer(const KeyServer&);
	KeyServer& operator= (const KeyServer&);
};

//-----------------------------------------------------------------------

#endif	// _KeyServer_H

