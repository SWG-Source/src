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

template <class T>
IteratedHashBase<T>::IteratedHashBase(unsigned int blockSize, unsigned int digestSize)
	: blockSize(blockSize), countLo(0), countHi(0)
	, data(blockSize/sizeof(T)), digest(digestSize/sizeof(T))
{
}

template <class T> void IteratedHashBase<T>::Update(const byte *input, unsigned int len)
{
	HashWordType tmp = countLo;
	if ((countLo = tmp + ((word32)len << 3)) < tmp)
		countHi++;             // Carry from low to high

   int rshift = (8*sizeof(HashWordType)-3);
   if (rshift<32)
   {
   	countHi += len >> rshift;
   }

	assert((blockSize & (blockSize-1)) == 0);	// blockSize is a power of 2
	unsigned int num = (unsigned int)(tmp >> 3) & (blockSize-1);

	if (num != 0)
	{
		if ((num+len) >= blockSize)
		{
			memcpy((byte *)data.ptr+num, input, blockSize-num);
			HashBlock(data);
			input += (blockSize-num);
			len-=(blockSize - num);
			num=0;
			// drop through and do the rest
		}
		else
		{
			memcpy((byte *)data.ptr+num, input, len);
			return;
		}
	}

	// we now can process the input data in blocks of blockSize
	// chars and save the leftovers to this->data.
	if (len >= blockSize)
	{
		if (IsAligned<T>(input))
		{
			unsigned int leftOver = HashMultipleBlocks((T *)input, len);
			input += (len - leftOver);
			len = leftOver;
		}
		else
			do
			{   // copy input first if it's not aligned correctly
				memcpy(data, input, blockSize);
				HashBlock(data);
				input+=blockSize;
				len-=blockSize;
			} while (len >= blockSize);
	}

	memcpy(data, input, len);
}

template <class T> unsigned int IteratedHashBase<T>::HashMultipleBlocks(const T *input, unsigned int length)
{
	do
	{
		HashBlock(input);
		input += blockSize/sizeof(T);
		length -= blockSize;
	}
	while (length >= blockSize);
	return length;
}

template <class T> void IteratedHashBase<T>::PadLastBlock(unsigned int lastBlockSize, byte padFirst)
{
	unsigned int num = (unsigned int)(countLo >> 3) & (blockSize-1);
	assert(num < blockSize);
	((byte *)data.ptr)[num++]=padFirst;
	if (num <= lastBlockSize)
		memset((byte *)data.ptr+num, 0, lastBlockSize-num);
	else
	{
		memset((byte *)data.ptr+num, 0, blockSize-num);
		HashBlock(data);
		memset(data, 0, lastBlockSize);
	}
}

template <class T> void IteratedHashBase<T>::Reinit()
{
	countLo = countHi = 0;
	Init();
}

// provide empty definitions to avoid instantiation warnings
template <class T> void IteratedHashBase<T>::Init() {}
template <class T> void IteratedHashBase<T>::HashBlock(const T *input) {}

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
		this->PadLastBlock(BLOCKSIZE - 2*sizeof(HashWordType));
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
