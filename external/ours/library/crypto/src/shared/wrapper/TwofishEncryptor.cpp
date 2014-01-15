// TwofishEncryptor.cpp
// copyright 2001 Verant Interactive
// Author: Justin Randall

//-----------------------------------------------------------------------
#include "FirstCrypto.h"
#include "md5.h"
#include "twofish.h"
#include "TwofishEncryptor.h"

namespace Crypto {

//-----------------------------------------------------------------------

TwofishEncryptor::TwofishEncryptor()
{
}

//-----------------------------------------------------------------------

TwofishEncryptor::TwofishEncryptor(const unsigned char * const keyData, const unsigned int keyLength)
{
	TwofishEncryptor::setKey(keyData, keyLength);
}

//-----------------------------------------------------------------------

TwofishEncryptor::TwofishEncryptor(const TwofishEncryptor &)
{
}

//-----------------------------------------------------------------------

TwofishEncryptor::~TwofishEncryptor()
{
}

//-----------------------------------------------------------------------

TwofishEncryptor & TwofishEncryptor::operator = (const TwofishEncryptor & rhs)
{
	if(this != &rhs)
	{
		// make assignments if right hand side is not this instance
	}
	return *this;
}

//-----------------------------------------------------------------------

void TwofishEncryptor::setKey(const unsigned char * const keyData, const unsigned int keyLength)
{
	delete cipher;
	
	// Twofish uses a 16 byte key length. MD5 digests are 16 bytes and are perfect for
	// whitening source data (keys) before use as initializing the encryptor
	CryptoPP::MD5 md5;
	unsigned char digest[16];
	md5.CalculateDigest(digest, keyData, keyLength);

	cipher = new CryptoPP::TwofishEncryption(keyData, keyLength);
}

//-----------------------------------------------------------------------


}//namespace Crypto

