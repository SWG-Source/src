// CryptoBufferTransform.h
// copyright 2001 Verant Interactive
// Author: Justin Randall

#ifndef	_INCLUDED_CryptoBufferTransform_H
#define	_INCLUDED_CryptoBufferTransform_H

namespace Crypto {

//-----------------------------------------------------------------------

class CryptoBufferTransform
{
public:
	CryptoBufferTransform();
	virtual ~CryptoBufferTransform();

	virtual const unsigned int getBlockSize () const = 0;
	virtual void               process      (const unsigned char * const inputBuffer, unsigned char * outputBuffer, const unsigned int size) = 0;
	virtual void               setKey       (const unsigned char * const keyData, const unsigned int keyLength) = 0;
private:
	CryptoBufferTransform & operator = (const CryptoBufferTransform & rhs);
	CryptoBufferTransform(const CryptoBufferTransform & source);

};

//-----------------------------------------------------------------------

}//namespace Crypto

#endif	// _INCLUDED_CryptoBufferTransform_H
