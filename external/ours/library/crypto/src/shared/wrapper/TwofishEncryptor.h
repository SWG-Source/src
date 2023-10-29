// TwofishEncryptor.h
// copyright 2001 Verant Interactive
// Author: Justin Randall

#ifndef	_INCLUDED_TwofishEncryptor_H
#define	_INCLUDED_TwofishEncryptor_H

//-----------------------------------------------------------------------

#include "twofish.h"
#include "TwofishCrypt.h"

namespace Crypto {

//-----------------------------------------------------------------------

class TwofishEncryptor : public TwofishCrypt<CryptoPP::TwofishEncryption>
{
public:
	TwofishEncryptor();
	TwofishEncryptor(const unsigned char * const keyData, const unsigned int keyLength);
	~TwofishEncryptor();

	void setKey(const unsigned char * const keyData, const unsigned int keyLength);
//	const unsigned int process(const unsigned char * const inputBuffer, unsigned char * outputBuffer, const unsigned int size);

private:
	TwofishEncryptor & operator = (const TwofishEncryptor & rhs);
	TwofishEncryptor(const TwofishEncryptor & source);
};

//-----------------------------------------------------------------------

}//namespace Crypto

#endif	// _INCLUDED_TwofishEncryptor_H
