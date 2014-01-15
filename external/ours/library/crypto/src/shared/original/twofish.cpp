// twofish.cpp - modified by Wei Dai from Matthew Skala's twofish.c
// The original code and all modifications are in the public domain.

#include "FirstCrypto.h"
#include "twofish.h"

NAMESPACE_BEGIN(CryptoPP)

// compute (c * x^4) mod (x^4 + (a + 1/a) * x^3 + a * x^2 + (a + 1/a) * x + 1)
// over GF(256)
static inline unsigned int Mod(unsigned int c)
{
	static const unsigned int modulus = 0x14d;
	unsigned int c2 = (c<<1) ^ ((c & 0x80) ? modulus : 0);
	unsigned int c1 = c2 ^ (c>>1) ^ ((c & 1) ? (modulus>>1) : 0);
	return c | (c1 << 8) | (c2 << 16) | (c1 << 24);
}

// compute RS(12,8) code with the above polynomial as generator
// this is equivalent to multiplying by the RS matrix
static word32 ReedSolomon(word32 high, word32 low)
{
	for (unsigned int i=0; i<8; i++)
	{
		high = Mod(high>>24) ^ (high<<8) ^ (low>>24);
		low <<= 8;
	}
	return high;
}

inline word32 Twofish::h0(word32 x, const word32 *key, unsigned int kLen)
{
	x = x | (x<<8) | (x<<16) | (x<<24);
	switch(kLen)
	{
#define Q(a, b, c, d, t) q[a][GETBYTE(t,0)] ^ (q[b][GETBYTE(t,1)] << 8) ^ (q[c][GETBYTE(t,2)] << 16) ^ (q[d][GETBYTE(t,3)] << 24)
	case 4: x = Q(1, 0, 0, 1, x) ^ key[6];
	case 3: x = Q(1, 1, 0, 0, x) ^ key[4];
	case 2: x = Q(0, 1, 0, 1, x) ^ key[2];
			x = Q(0, 0, 1, 1, x) ^ key[0];
	}
	return x;
}

inline word32 Twofish::h(word32 x, const word32 *key, unsigned int kLen)
{
	x = h0(x, key, kLen);
	return mds[0][GETBYTE(x,0)] ^ mds[1][GETBYTE(x,1)] ^ mds[2][GETBYTE(x,2)] ^ mds[3][GETBYTE(x,3)];
}

Twofish::Twofish(const byte *userKey, unsigned int keylength)
	: m_k(40), m_s(4)
{
	assert(keylength == KeyLength(keylength));

	unsigned int len = (keylength <= 16 ? 2 : (keylength <= 24 ? 3 : 4));
	SecBlock<word32> key(len*2);
	GetUserKeyLittleEndian(key.ptr, len*2, userKey, keylength);

	unsigned int i;
	for (i=0; i<40; i+=2)
	{
		word32 a = h(i, key, len);
		word32 b = rotlFixed(h(i+1, key+1u, len), 8);
		m_k[i] = a+b;
		m_k[i+1] = rotlFixed(a+2*b, 9);
	}

	SecBlock<word32> svec(2*len);
	for (i=0; i<len; i++)
		svec[2*(len-i-1)] = ReedSolomon(key[2*i+1], key[2*i]);
	for (i=0; i<256; i++)
	{
		word32 t = h0(i, svec, len);
		m_s[0u][i] = mds[0][GETBYTE(t, 0)];
		m_s[1u][i] = mds[1][GETBYTE(t, 1)];
		m_s[2u][i] = mds[2][GETBYTE(t, 2)];
		m_s[3u][i] = mds[3][GETBYTE(t, 3)];
	}
}

#define G1(x) (m_s[0u][GETBYTE(x,0)] ^ m_s[1u][GETBYTE(x,1)] ^ m_s[2u][GETBYTE(x,2)] ^ m_s[3u][GETBYTE(x,3)])
#define G2(x) (m_s[0u][GETBYTE(x,3)] ^ m_s[1u][GETBYTE(x,0)] ^ m_s[2u][GETBYTE(x,1)] ^ m_s[3u][GETBYTE(x,2)])

#define ENCROUND(n, a, b, c, d) \
	x = G1 (a); y = G2 (b); \
	x += y; y += x + k[2 * (n) + 1]; \
	(c) ^= x + k[2 * (n)]; \
	(c) = rotrFixed(c, 1); \
	(d) = rotlFixed(d, 1) ^ y

#define ENCCYCLE(n) \
	ENCROUND (2 * (n), a, b, c, d); \
	ENCROUND (2 * (n) + 1, c, d, a, b)

#define DECROUND(n, a, b, c, d) \
	x = G1 (a); y = G2 (b); \
	x += y; y += x; \
	(d) ^= y + k[2 * (n) + 1]; \
	(d) = rotrFixed(d, 1); \
	(c) = rotlFixed(c, 1); \
	(c) ^= (x + k[2 * (n)])

#define DECCYCLE(n) \
	DECROUND (2 * (n) + 1, c, d, a, b); \
	DECROUND (2 * (n), a, b, c, d)

void TwofishEncryption::ProcessBlock(const byte *inBlock, byte *outBlock) const
{
	word32 x, y, a, b, c, d;

	GetBlockLittleEndian(inBlock, a, b, c, d);

	a ^= m_k[0u];
	b ^= m_k[1u];
	c ^= m_k[2u];
	d ^= m_k[3u];

	const word32 *k = m_k+8u;
	ENCCYCLE (0);
	ENCCYCLE (1);
	ENCCYCLE (2);
	ENCCYCLE (3);
	ENCCYCLE (4);
	ENCCYCLE (5);
	ENCCYCLE (6);
	ENCCYCLE (7);

	c ^= m_k[4u];
	d ^= m_k[5u];
	a ^= m_k[6u];
	b ^= m_k[7u]; 

	PutBlockLittleEndian(outBlock, c, d, a, b);
}

void TwofishDecryption::ProcessBlock(const byte *inBlock, byte *outBlock) const
{
	word32 x, y, a, b, c, d;

	GetBlockLittleEndian(inBlock, c, d, a, b);

	c ^= m_k[4u];
	d ^= m_k[5u];
	a ^= m_k[6u];
	b ^= m_k[7u];

	const word32 *k = m_k+8u;
	DECCYCLE (7);
	DECCYCLE (6);
	DECCYCLE (5);
	DECCYCLE (4);
	DECCYCLE (3);
	DECCYCLE (2);
	DECCYCLE (1);
	DECCYCLE (0);

	a ^= m_k[0u];
	b ^= m_k[1u];
	c ^= m_k[2u];
	d ^= m_k[3u];

	PutBlockLittleEndian(outBlock, a, b, c, d);
}

NAMESPACE_END
