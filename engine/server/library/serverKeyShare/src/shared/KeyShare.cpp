// KeyShare.cpp
// copyright 2000 Verant Interactive
// Author: Justin Randall

//-----------------------------------------------------------------------

#include "serverKeyShare/FirstServerKeyShare.h"
#include "MD5Hash.h"
#include "TwofishDecryptor.h"
#include "TwofishEncryptor.h"
#include "serverKeyShare/KeyShare.h"

//-----------------------------------------------------------------------

KeyShare::Token::Token(const unsigned char * const newCipherData,
	const uint32 newDataLen,
	const unsigned char newDigest[KeyShareConstants::keyLength],
	const uint32 newCipherDataLen) :
	cipherData(0),
	cipherDataLen(newCipherDataLen),
	dataLen(newDataLen)
{
	cipherData = new unsigned char[cipherDataLen];
	memcpy(cipherData, newCipherData, cipherDataLen);
	memcpy(digest, newDigest, KeyShareConstants::keyLength);
}

//-----------------------------------------------------------------------

KeyShare::Token::Token(const Token & source) :
	cipherData(0),
	cipherDataLen(source.cipherDataLen),
	dataLen(source.dataLen)
{
	cipherData = new unsigned char[cipherDataLen];
	memcpy(cipherData, source.cipherData, cipherDataLen);
	memcpy(digest, source.digest, KeyShareConstants::keyLength);
}

//-----------------------------------------------------------------------

KeyShare::Token::Token(Archive::ReadIterator & source) :
	cipherData(0),
	cipherDataLen(0),
	dataLen(0)
{
	Archive::get(source, cipherDataLen);

	// if the cipher data has been tampered with, don't unpack it. The vaidation will fail
	if (cipherDataLen < 64)
	{
		Archive::get(source, dataLen);
		cipherData = new unsigned char[cipherDataLen];
		unsigned int i;
		for (i = 0; i < cipherDataLen; i++)
		{
			Archive::get(source, cipherData[i]);
		}
		for (i = 0; i < KeyShareConstants::keyLength; i++)
		{
			Archive::get(source, digest[i]);
		}
	}
	else
	{
		cipherData = new unsigned char[64];
		cipherDataLen = 64;
		dataLen = 0;
	}
}

//-----------------------------------------------------------------------

KeyShare::Token::~Token()
{
	delete[] cipherData;
}

//-----------------------------------------------------------------------

const uint32 KeyShare::Token::getCipherDataLen(void) const
{
	return cipherDataLen;
}

//-----------------------------------------------------------------------

const unsigned char * const KeyShare::Token::getData(void) const
{
	return cipherData;
}

//-----------------------------------------------------------------------

const uint32 KeyShare::Token::getDataLen(void) const
{
	return dataLen;
}

//-----------------------------------------------------------------------

const unsigned char * const KeyShare::Token::getDigest(void) const
{
	return digest;
}

//-----------------------------------------------------------------------

void KeyShare::Token::pack(Archive::ByteStream & target) const
{
	Archive::put(target, cipherDataLen);
	Archive::put(target, dataLen);
	unsigned int i;
	for (i = 0; i < cipherDataLen; i++)
	{
		Archive::put(target, cipherData[i]);
	}
	for (i = 0; i < KeyShareConstants::keyLength; i++)
	{
		Archive::put(target, digest[i]);
	}
}

//-----------------------------------------------------------------------

KeyShare::KeyShare(const unsigned int newKeyCount) :
	decryptors(0),
	encryptors(0),
	hasher(0),
	keyCount(newKeyCount),
	keys(0)
{
	keys = new Key[keyCount];
	decryptors = new Crypto::TwofishDecryptor *[keyCount];
	encryptors = new Crypto::TwofishEncryptor *[keyCount];

	for (unsigned int i = 0; i < keyCount; i++)
	{
		decryptors[i] = 0;
		encryptors[i] = 0;
	}
	hasher = new Crypto::MD5Hash;
}

//-----------------------------------------------------------------------

KeyShare::~KeyShare()
{
	delete[] keys;
	for (unsigned int i = 0; i < keyCount; i++)
	{
		delete decryptors[i];
		delete encryptors[i];
	}

	delete[] decryptors;
	delete[] encryptors;
	delete hasher;
}

//-----------------------------------------------------------------------

bool KeyShare::decipherToken(const KeyShare::Token & token, unsigned char * clearTextData, uint32 & dataLen) const
{
	bool result = false;
	if (dataLen < token.getDataLen())
	{
		dataLen = token.getDataLen();
	}
	else
	{
		unsigned int	cipherDataLen = token.getCipherDataLen();
		unsigned char * cipherText = new unsigned char[cipherDataLen];
		unsigned char * clearText = new unsigned char[cipherDataLen];

		memcpy(cipherText, token.getData(), cipherDataLen);
		for (unsigned int i = 0; i < keyCount; i++)
		{
			DEBUG_REPORT_LOG(true, ("Decrypting with key: "));
			for (uint32 j = 0; j < 16; j++)
			{
				DEBUG_REPORT_LOG(true, ("[%3i] ", keys[i].value[j]));
			}
			DEBUG_REPORT_LOG(true, ("\n"));
			// find key index
			unsigned char	digest[KeyShareConstants::keyLength];
			decrypt(token.getDigest(), KeyShareConstants::keyLength, digest, i);
			dataLen = token.getDataLen();
			decrypt(cipherText, cipherDataLen, clearText, i);
			memcpy(clearTextData, clearText, dataLen);

			NOT_NULL(hasher);

			result = hasher->verify(digest, clearTextData, dataLen);
			if (result)
			{
				DEBUG_REPORT_LOG(true, ("succeeded\n"));

				break;
			}
			else
			{
				DEBUG_REPORT_LOG(true, ("failed\n"));
			}
		}
		delete[] cipherText;
		delete[] clearText;
	}
	return result;
}

//-----------------------------------------------------------------------

void KeyShare::decrypt(const unsigned char * const sourceBuffer, const uint32 sourceBufferSize, unsigned char * resultBuffer, const uint32 keyIndex) const
{
	const uint32 blockSize = decryptors[0]->getBlockSize();
	unsigned char * inBlock = new unsigned char[blockSize];
	unsigned char * outBlock = new unsigned char[blockSize];

	memset(resultBuffer, 0, sourceBufferSize);

	for (uint32 i = 0; i < sourceBufferSize; i += blockSize)
	{
		if (i + blockSize > sourceBufferSize)
			memcpy(inBlock, &sourceBuffer[i], sourceBufferSize - i);
		else
			memcpy(inBlock, &sourceBuffer[i], blockSize);
		decryptors[keyIndex]->process(inBlock, outBlock, blockSize);
		memcpy(&resultBuffer[i], outBlock, blockSize);
	}
	delete[] inBlock;
	delete[] outBlock;
}

//-----------------------------------------------------------------------

const uint32 KeyShare::encrypt(const unsigned char * const sourceData, const uint32 sourceDataSize, unsigned char ** resultBuffer) const
{
	uint32 i;
	const uint32 blockSize = encryptors[0]->getBlockSize();

	// calculate the size of the encrypted result buffer
	if (sourceDataSize % blockSize)
	{
		// if it's larger than some multiple of the cipher block size ....
		i = static_cast<uint32>((sourceDataSize / blockSize) + 1) * blockSize;
	}
	else
	{
		// it's already aligned on the cipher block byte boundary
		i = sourceDataSize;
	}
	*resultBuffer = new unsigned char[i];

	DEBUG_REPORT_LOG(true, ("Encrypting with key: "));
	for (i = 0; i < 16; i++)
	{
		DEBUG_REPORT_LOG(true, ("[%3i] ", keys[0].value[i]));
	}
	DEBUG_REPORT_LOG(true, ("\n"));

	unsigned char * inBlock = new unsigned char[blockSize];
	unsigned char * outBlock = new unsigned char[blockSize];

	for (i = 0; i < sourceDataSize; i += 16)
	{
		memset(inBlock, 0, KeyShareConstants::keyLength);
		if (i + blockSize > sourceDataSize)
			memcpy(inBlock, &sourceData[i], sourceDataSize - i);
		else
			memcpy(inBlock, &sourceData[i], KeyShareConstants::keyLength);
		encryptors[0]->process(inBlock, outBlock, blockSize);
		memcpy(&((*resultBuffer)[i]), outBlock, blockSize);
	}

	delete[] inBlock;
	delete[] outBlock;
	return i;
}

//-----------------------------------------------------------------------

bool KeyShare::hasKey(const Key & source)
{
	for (unsigned int i = 0; i < getKeyCount(); i++)
	{
		if (memcmp(source.value, keys[i].value, KeyShareConstants::keyLength) == 0)
			return true;
	}
	return false;
}

//-----------------------------------------------------------------------

KeyShare::Token KeyShare::makeToken(const unsigned char * const data, const uint32 dataLen) const
{
	// generate a 128 bit message digest representing the data
	unsigned char digest[KeyShareConstants::keyLength];

	NOT_NULL(hasher);

	hasher->process(data, digest, dataLen);

	// encrypt the digest
	unsigned char * cipherDigest;
	IGNORE_RETURN(encrypt(digest, KeyShareConstants::keyLength, &cipherDigest)); ///@todo error checking

	// encrypt the cleartext data
	unsigned char * cipherData = 0;
	uint32 cipherDataLen = encrypt(data, dataLen, &cipherData);

	// build a token using the ciphertext data and digest
	Token	token(cipherData, dataLen, cipherDigest, cipherDataLen);

	// clean up allocation for cipher text,
	// it was deep copied in the token
	delete[] cipherData;
	delete[] cipherDigest;

	return token;
}

//-----------------------------------------------------------------------

void KeyShare::pushKey(const Key & source)
{
	shift();
	setKey(source, 0);
}

//-----------------------------------------------------------------------

void KeyShare::setKey(const Key & newKey, const unsigned int index)
{
	DEBUG_FATAL(index >= keyCount, ("Key index out of range\n"));
	keys[index] = newKey;
	delete decryptors[index];
	delete encryptors[index];
	decryptors[index] = new Crypto::TwofishDecryptor(newKey.value, KeyShareConstants::keyLength);
	encryptors[index] = new Crypto::TwofishEncryptor(newKey.value, KeyShareConstants::keyLength);
}

//-----------------------------------------------------------------------

void KeyShare::shift(void)
{
	// shift key array
	for (unsigned int i = getKeyCount() - 1; i > 0; i--)
	{
		setKey(getKey(i - 1), i);
	}
}

//-----------------------------------------------------------------------

AutoVariableKeyShare::AutoVariableKeyShare() :
	AutoVariableBase(),
	value()
{
}

//-----------------------------------------------------------------------

AutoVariableKeyShare::AutoVariableKeyShare(const KeyShare::Key & source) :
	AutoVariableBase()
{
	value = source;
}

//-----------------------------------------------------------------------

AutoVariableKeyShare::~AutoVariableKeyShare()
{
}

//-----------------------------------------------------------------------

AutoVariableKeyShare & AutoVariableKeyShare::operator=(const KeyShare::Key & source)
{
	value = source;
	return *this;
}

//-----------------------------------------------------------------------

AutoVariableKeyShare::operator KeyShare::Key & ()
{
	return value;
}

//-----------------------------------------------------------------------

AutoVariableKeyShare::operator const KeyShare::Key & () const
{
	return value;
}

//-----------------------------------------------------------------------

void AutoVariableKeyShare::pack(Archive::ByteStream & target) const
{
	for (unsigned int i = 0; i < KeyShareConstants::keyLength; ++i)
		Archive::put(target, value.value[i]);
}

//-----------------------------------------------------------------------

void AutoVariableKeyShare::unpack(Archive::ReadIterator & source)
{
	for (unsigned int i = 0; i < KeyShareConstants::keyLength; ++i)
		Archive::get(source, value.value[i]);
}

//-----------------------------------------------------------------------