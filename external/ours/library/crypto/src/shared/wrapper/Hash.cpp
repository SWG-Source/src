// Hash.cpp
// copyright 2001 Verant Interactive
// Author: Justin Randall

//-----------------------------------------------------------------------

#include "FirstCrypto.h"
#include "Hash.h"

namespace Crypto {

//-----------------------------------------------------------------------

Hash::Hash()
{
}

//-----------------------------------------------------------------------

Hash::Hash(const Hash &)
{
}

//-----------------------------------------------------------------------

Hash::~Hash()
{
}

//-----------------------------------------------------------------------

Hash & Hash::operator = (const Hash & rhs)
{
	if(this != &rhs)
	{
		// make assignments if right hand side is not this instance
	}
	return *this;
}

//-----------------------------------------------------------------------

}//namspace Crypto
