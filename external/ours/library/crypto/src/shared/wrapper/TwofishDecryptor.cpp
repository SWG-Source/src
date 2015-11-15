// TwofishDecryptor.cpp
// copyright 2001 Verant Interactive
// Author: Justin Randall

//-----------------------------------------------------------------------

#include "FirstCrypto.h"
#include "md5.h"
#include "twofish.h"
#include "TwofishDecryptor.h"

namespace Crypto {

//-----------------------------------------------------------------------

TwofishDecryptor::TwofishDecryptor()
{
}

//-----------------------------------------------------------------------

TwofishDecryptor::TwofishDecryptor(const unsigned char * const keyData, const unsigned int keyLength)
{
	TwofishDecryptor::setKey(keyData, keyLength);
}

//-----------------------------------------------------------------------

TwofishDecryptor::TwofishDecryptor(const TwofishDecryptor &)
{
}

//-----------------------------------------------------------------------

TwofishDecryptor::~TwofishDecryptor()
{
}

//-----------------------------------------------------------------------

TwofishDecryptor & TwofishDecryptor::operator = (const TwofishDecryptor & rhs)
{
	if(this != &rhs)
	{
		// make assignments if right hand side is not this instance
	}
	return *this;
}

//-----------------------------------------------------------------------

void TwofishDecryptor::setKey(const unsigned char * const keyData, const unsigned int keyLength)
{
	delete cipher;
	
	// Twofish uses a 16 byte key length. MD5 digests are 16 bytes and are perfect for
	// whitening source data (keys) before use as initializing the encryptor
	CryptoPP::MD5 md5;
	unsigned char digest[16];
	md5.CalculateDigest(digest, keyData, keyLength);

	cipher = new CryptoPP::TwofishDecryption(keyData, keyLength);
}

//-----------------------------------------------------------------------

}//namespace Crypto

