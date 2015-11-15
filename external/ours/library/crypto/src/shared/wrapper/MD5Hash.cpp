// MD5Hash.cpp
// copyright 2001 Verant Interactive
// Author: Justin Randall

//-----------------------------------------------------------------------
#include "FirstCrypto.h"
#include "md5.h"
#include "MD5Hash.h"

namespace Crypto {

//-----------------------------------------------------------------------

MD5Hash::MD5Hash() :
md5(new CryptoPP::MD5)
{
}

//-----------------------------------------------------------------------

MD5Hash::MD5Hash(const MD5Hash &)
{
}

//-----------------------------------------------------------------------

MD5Hash::~MD5Hash()
{
	delete md5;
}

//-----------------------------------------------------------------------

MD5Hash & MD5Hash::operator = (const MD5Hash & rhs)
{
	if(this != &rhs)
	{
		// make assignments if right hand side is not this instance
	}
	return *this;
}

//-----------------------------------------------------------------------

const unsigned int MD5Hash::getHashSize() const
{
	return HASHSIZE;
}

//-----------------------------------------------------------------------

void MD5Hash::process(const unsigned char * const inputBuffer, unsigned char * outputBuffer, const unsigned int inputBufferSize)
{
	md5->CalculateDigest(outputBuffer, inputBuffer, inputBufferSize);
}

//-----------------------------------------------------------------------

bool MD5Hash::verify(const unsigned char * const digest, const unsigned char * const data, const unsigned int dataLen)
{
	return md5->VerifyDigest(digest, data, dataLen);
}

//-----------------------------------------------------------------------

}//namespace Crypto
