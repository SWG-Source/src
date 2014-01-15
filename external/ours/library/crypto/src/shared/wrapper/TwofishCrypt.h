// TwofishCrypt.h
// copyright 2001 Verant Interactive
// Author: Justin Randall

#ifndef	_INCLUDED_TwofishCrypt_H
#define	_INCLUDED_TwofishCrypt_H

//-----------------------------------------------------------------------

#include "CryptoBufferTransform.h"

namespace CryptoPP
{
	class Twofish;
}

namespace Crypto {

//-----------------------------------------------------------------------
/**
	@brief abstract base class for Twofish ciphers
*/
class TwofishCrypt
{
public:
	TwofishCrypt();
	virtual ~TwofishCrypt() = 0;

	enum { BLOCKSIZE=16 };
	const unsigned int getBlockSize() const;
	void process(const unsigned char * const inputBuffer, unsigned char * outputBuffer, const unsigned int size);

private:
	TwofishCrypt & operator = (const TwofishCrypt & rhs);
	TwofishCrypt(const TwofishCrypt & source);

protected:
	/** @brief assigned by a TwofishEncryptor or TwofishDecryptor */
	CryptoPP::Twofish *      cipher;
};

//-----------------------------------------------------------------------

}// namespace Crypto

#endif	// _INCLUDED_TwofishCrypt_H
