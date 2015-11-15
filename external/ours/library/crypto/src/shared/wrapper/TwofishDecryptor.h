// TwofishDecryptor.h
// copyright 2001 Verant Interactive
// Author: Justin Randall

#ifndef	_INCLUDED_TwofishDecryptor_H
#define	_INCLUDED_TwofishDecryptor_H

//-----------------------------------------------------------------------

#include "TwofishCrypt.h"

namespace Crypto {

//-----------------------------------------------------------------------

	class TwofishDecryptor : public TwofishCrypt
{
public:
	TwofishDecryptor();
	TwofishDecryptor(const unsigned char * const keyData, const unsigned int keyLength);
	~TwofishDecryptor();

	void setKey(const unsigned char * const keyData, const unsigned int keyLength);

private:
	TwofishDecryptor & operator = (const TwofishDecryptor & rhs);
	TwofishDecryptor(const TwofishDecryptor & source);

};

//-----------------------------------------------------------------------

}// namespace Crypto

#endif	// _INCLUDED_TwofishDecryptor_H
