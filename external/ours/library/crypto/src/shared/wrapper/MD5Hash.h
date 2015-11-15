// MD5Hash.h
// copyright 2001 Verant Interactive
// Author: Justin Randall

#ifndef	_INCLUDED_MD5Hash_H
#define	_INCLUDED_MD5Hash_H

//-----------------------------------------------------------------------

#include "Hash.h"

namespace CryptoPP
{
	class MD5;
}

namespace Crypto {

//-----------------------------------------------------------------------

class MD5Hash
{
public:
	MD5Hash();
	~MD5Hash();
	
	enum { HASHSIZE = 16 };

	const unsigned int getHashSize() const;
	void               process(const unsigned char * const inputBuffer, unsigned char * outputBuffer, const unsigned int inputBufferSize);
	bool               verify(const unsigned char * const digest, const unsigned char * const data, const unsigned int dataLen);

private:
	MD5Hash & operator = (const MD5Hash & rhs);
	MD5Hash(const MD5Hash & source);

private:
	CryptoPP::MD5 *	md5;
};

//-----------------------------------------------------------------------

}//namespace Crypto

#endif	// _INCLUDED_MD5Hash_H
