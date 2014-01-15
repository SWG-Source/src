// KeyShare.h
// copyright 2000 Verant Interactive
// Author: Justin Randall

#ifndef	_KeyShare_H
#define	_KeyShare_H

//-----------------------------------------------------------------------

#include "Archive/AutoByteStream.h"

namespace Crypto
{
	class TwofishEncryptor;
	class TwofishDecryptor;
	class MD5Hash;
}

namespace KeyShareConstants
{
	const unsigned int	keyLength	= 16;
}

//-----------------------------------------------------------------------

class KeyShare
{
public:
	struct Key
	{
		unsigned char	value[KeyShareConstants::keyLength];
	};

	/**
		An instance of a token ALWAYS contains encrypted data.
	*/
	class Token 
	{
	public:
		Token(const unsigned char * const cipherData, const uint32 dataLen, const unsigned char digest[KeyShareConstants::keyLength], const uint32 cipherDataLen);
		Token(const Token & source);
		explicit Token(Archive::ReadIterator & source);
		~Token();
		Token& operator = (const Token & source);
		const uint32				getCipherDataLen(void) const;
		const unsigned char * const	getData(void) const;
		const uint32				getDataLen(void) const;
		const unsigned char * const	getDigest(void) const;
		void                        pack(Archive::ByteStream & target) const;
	private:
		unsigned char * cipherData;
		uint32          cipherDataLen;
		uint32          dataLen;
		unsigned char   digest[KeyShareConstants::keyLength];
		Token();
	};

public:
	explicit KeyShare(const unsigned int newKeyCount=2);
	virtual ~KeyShare();

	bool				decipherToken		(const Token & token, unsigned char * clearTextData, uint32 & dataLen) const;
	const Key&			getKey				(const unsigned int index) const;
	const unsigned int	getKeyCount			(void) const;
	bool				hasKey				(const Key & source);
	Token				makeToken			(const unsigned char * const data, const uint32 dataLen) const;
	const bool			isTokenValid		(const Token & token) const;
	void				pushKey				(const Key & source);
	void				setKey				(const Key & newKey, const unsigned int index);
	const Key &			operator[]			(const unsigned int index) const;
private:
	/**
	*/
	void				decrypt				(const unsigned char * const sourceData, const uint32 sourceDataSize, unsigned char * resultBuffer, const uint32 keyIndex) const;

	/**
	*/
	const uint32		encrypt				(const unsigned char * const sourceData, const uint32 sourceDataSize, unsigned char ** resultBuffer) const;
	void				shift(void);

	Crypto::TwofishDecryptor **	decryptors;
	Crypto::TwofishEncryptor **	encryptors;	
	Crypto::MD5Hash *		hasher;
	unsigned int		keyCount;
	Key *				keys;


	KeyShare(const KeyShare&);
	KeyShare& operator= (const KeyShare&);
};

//-----------------------------------------------------------------------

inline const KeyShare::Key & KeyShare::operator[](const unsigned int index) const
{
	DEBUG_FATAL(index >= keyCount, ("Key index out of range\n"));
	return getKey(index);
}

//-----------------------------------------------------------------------

inline const unsigned int KeyShare::getKeyCount(void) const
{
	return keyCount;
}

//-----------------------------------------------------------------------

inline const KeyShare::Key& KeyShare::getKey(const unsigned int index) const 
{												//lint !e1727 //not defined as inline? @todo 
	DEBUG_FATAL(index >= keyCount, ("Key index out of range\n"));
	return keys[index];
}

//-----------------------------------------------------------------------
/**
	@brief Extend archive system to support the keyshare key data type
*/
class AutoVariableKeyShare : public Archive::AutoVariableBase
{
public:
	AutoVariableKeyShare();
	explicit AutoVariableKeyShare(const KeyShare::Key & source);
	~AutoVariableKeyShare();
	AutoVariableKeyShare & operator=(const KeyShare::Key & rhs);
	AutoVariableKeyShare & operator=(const AutoVariableKeyShare & rhs);

	operator KeyShare::Key & (); //lint !e1930 // this is the intended behavior of an auto variable
	operator const KeyShare::Key & () const; //lint !e1930 // this is the intended behavior of an auto variable

	virtual void  pack(Archive::ByteStream & target) const;
	virtual void  unpack(Archive::ReadIterator & source);
private:
	KeyShare::Key	value;
};

//-----------------------------------------------------------------------

#endif	//_KeyShare_H

