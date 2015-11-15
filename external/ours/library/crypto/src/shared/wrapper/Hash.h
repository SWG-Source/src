// Hash.h
// copyright 2001 Verant Interactive
// Author: Justin Randall

#ifndef	_INCLUDED_Hash_H
#define	_INCLUDED_Hash_H

//-----------------------------------------------------------------------

namespace Crypto {

//-----------------------------------------------------------------------

class Hash
{
public:
	Hash();
	virtual ~Hash();

	virtual const unsigned int getHashSize() const = 0;
	virtual void               process(const unsigned char * const inputBuffer, unsigned char * outputBuffer, const unsigned int inputBufferSize) = 0;
	virtual bool               verify(const unsigned char * const digest, const unsigned char * const data, const unsigned int dataLen) = 0;
private:
	Hash & operator = (const Hash & rhs);
	Hash(const Hash & source);

};

//-----------------------------------------------------------------------

}//namespace Crypto

#endif	// _INCLUDED_Hash_H
