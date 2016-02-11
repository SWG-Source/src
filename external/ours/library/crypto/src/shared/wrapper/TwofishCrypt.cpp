// TwofishCrypt.cpp
// copyright 2001 Verant Interactive
// Author: Justin Randall

//-----------------------------------------------------------------------

#include "FirstCrypto.h"
#include "twofish.h"
#include "TwofishCrypt.h"

namespace Crypto {

//-----------------------------------------------------------------------
/**
	@brief default ctor
*/
TwofishCrypt::TwofishCrypt() :
cipher(0)
{
}

//-----------------------------------------------------------------------

TwofishCrypt::TwofishCrypt(const TwofishCrypt &)
{
}

//-----------------------------------------------------------------------
/**
	@brief dtor

	Deletes the instance of the Twofish cipher
*/
TwofishCrypt::~TwofishCrypt()
{
	delete cipher;
}

//-----------------------------------------------------------------------
/**
	@brief disabled assignment operator
*/
TwofishCrypt & TwofishCrypt::operator = (const TwofishCrypt &)
{
	return *this;
}

//-----------------------------------------------------------------------
/**
	@brief return the blocksize of the cipher
*/
const unsigned int TwofishCrypt::getBlockSize() const
{
	return BLOCKSIZE;
}

//-----------------------------------------------------------------------
/**
	@brief perform a cipher operation

	The cipher operation is determined by the instance of the 
	TwofishCrypt object (which is actually either a TwofishEncryptor or 
	TwofishDecryptor)

	The input block size must be greater than or equal to the cipher block
	size.
*/
void TwofishCrypt::process(const unsigned char * const inputBuffer, unsigned char * outputBuffer, const unsigned int size)
{
	static unsigned char block[BLOCKSIZE];

	if(cipher && size >= BLOCKSIZE)
	{
		const unsigned int r = size & 3; // optimization -- safe is % BLOCKSIZE
		if(r == 0)
		{
			unsigned int i;
			for(i = 0; i < size; i += BLOCKSIZE)
			{
				memcpy(block, &inputBuffer[i], BLOCKSIZE);
				cipher->ProcessBlock(block);
				memcpy(&outputBuffer[i], block, BLOCKSIZE);
			}
		}
		assert( r == 0 );	// size must be a 16 byte block for Twofish to do it's job!
	}
	assert(cipher != nullptr); // can't process data without a twofish encryptor or decryptor!
}

//-----------------------------------------------------------------------

}//namespace Crypto

