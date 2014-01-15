#ifndef CRYPTOPP_TWOFISH_H
#define CRYPTOPP_TWOFISH_H

/** \file
*/

#include "cryptlib.h"
#include "misc.h"

NAMESPACE_BEGIN(CryptoPP)

/// base class, do not use directly
class Twofish : public FixedBlockSize<16>, public VariableKeyLength<16, 0, 32>
{
protected:
	Twofish(const byte *userKey, unsigned int keylength);
	static word32 h0(word32 x, const word32 *key, unsigned int kLen);
	static word32 h(word32 x, const word32 *key, unsigned int kLen);

	static const byte q[2][256];
	static const word32 mds[4][256];

	SecBlock<word32> m_k;
	SecBlock<word32[256]> m_s;
};

/// <a href="http://www.weidai.com/scan-mirror/cs.html#Twofish">Twofish</a>
class TwofishEncryption : public Twofish
{
public:
	TwofishEncryption(const byte *userKey, unsigned int keylength=DEFAULT_KEYLENGTH)
		: Twofish(userKey, keylength) {}

	void ProcessBlock(const byte *inBlock, byte * outBlock) const;
	void ProcessBlock(byte * inoutBlock) const
		{TwofishEncryption::ProcessBlock(inoutBlock, inoutBlock);}
};

/// <a href="http://www.weidai.com/scan-mirror/cs.html#Twofish">Twofish</a>
class TwofishDecryption : public Twofish
{
public:
	TwofishDecryption(const byte *userKey, unsigned int keylength=DEFAULT_KEYLENGTH)
		: Twofish(userKey, keylength) {}

	void ProcessBlock(const byte *inBlock, byte * outBlock) const;
	void ProcessBlock(byte * inoutBlock) const
		{TwofishDecryption::ProcessBlock(inoutBlock, inoutBlock);}
};

NAMESPACE_END

#endif
