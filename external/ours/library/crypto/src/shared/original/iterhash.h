#ifndef CRYPTOPP_ITERHASH_H
#define CRYPTOPP_ITERHASH_H

#include "cryptlib.h"
#include "misc.h"

NAMESPACE_BEGIN(CryptoPP)

/*! The following classes are explicitly instantiated in iterhash.cpp

	IteratedHashBase<word32>
	IteratedHashBase<word64>	// #ifdef WORD64_AVAILABLE
*/
template <class T>
class IteratedHashBase : public virtual HashModule
{
public:
	typedef T HashWordType;

	IteratedHashBase(unsigned int blockSize, unsigned int digestSize);
	unsigned int DigestSize() const {return digest.size * sizeof(T);};
	void Update(const byte *input, unsigned int length);

protected:
	virtual unsigned int HashMultipleBlocks(const T *input, unsigned int length);
	void PadLastBlock(unsigned int lastBlockSize, byte padFirst=0x80);
	void Reinit();
	virtual void Init() =0;
	virtual void HashBlock(const T *input) =0;

	unsigned int blockSize;
	word32 countLo, countHi;	// 64-bit bit count
	SecBlock<T> data;			// Data buffer
	SecBlock<T> digest;			// Message digest
};

//! .
template <class T, bool H, unsigned int S>
class IteratedHash : public IteratedHashBase<T>
{
public:
	typedef T HashWordType;
	enum {HIGHFIRST = H, BLOCKSIZE = S};
	
	IteratedHash(unsigned int digestSize) : IteratedHashBase<T>(BLOCKSIZE, digestSize) {}

	inline static void CorrectEndianess(HashWordType *out, const HashWordType *in, unsigned int byteCount)
	{
		if (!CheckEndianess(HIGHFIRST))
			byteReverse(out, in, byteCount);
		else if (in!=out)
			memcpy(out, in, byteCount);
	}

	void Final(byte *hash)
	{
		PadLastBlock(BLOCKSIZE - 2*sizeof(HashWordType));
		CorrectEndianess(this->data, this->data, BLOCKSIZE - 2*sizeof(HashWordType));

		this->data[this->data.size-2] = HIGHFIRST ? this->countHi : this->countLo;
		this->data[this->data.size-1] = HIGHFIRST ? this->countLo : this->countHi;

		vTransform(this->data);
		CorrectEndianess(this->digest, this->digest, this->DigestSize());
		memcpy(hash, this->digest, this->DigestSize());

		this->Reinit();		// reinit for next use
	}

protected:
	void HashBlock(const HashWordType *input)
	{
		if (CheckEndianess(HIGHFIRST))
			vTransform(input);
		else
		{
			byteReverse(this->data.ptr, input, (unsigned int)BLOCKSIZE);
			vTransform(this->data);
		}
	}

	virtual void vTransform(const HashWordType *data) =0;
};

NAMESPACE_END

#endif
