// TwofishCrypt.h
// copyright 2001 Verant Interactive
// Author: Justin Randall

#ifndef	_INCLUDED_TwofishCrypt_H
#define	_INCLUDED_TwofishCrypt_H

//-----------------------------------------------------------------------

#include "assert.h"
#include "CryptoBufferTransform.h"

namespace CryptoPP
{
	class Twofish;
}

namespace Crypto {

//-----------------------------------------------------------------------
/**
	@brief abstract base class for Twofish ciphers
*/
template <typename T>
class TwofishCrypt
{
public:
    TwofishCrypt() : cipher(0)
    {
    }

    virtual ~TwofishCrypt() = 0;

    enum
    {
        BLOCKSIZE = 16
    };

    const unsigned int getBlockSize() const
    {
        return BLOCKSIZE;
    }

    void process(const unsigned char *const inputBuffer, unsigned char *outputBuffer, const unsigned int size)
    {
        static unsigned char block[BLOCKSIZE];

        if (cipher && size >= BLOCKSIZE)
        {
            const unsigned int r = size & 3; // optimization -- safe is % BLOCKSIZE
            if (r == 0)
            {
                unsigned int i;
                for (i = 0; i < size; i += BLOCKSIZE)
                {
                    memcpy(block, &inputBuffer[i], BLOCKSIZE);
                    cipher->ProcessAndXorBlock(block, NULLPTR, block);
                    memcpy(&outputBuffer[i], block, BLOCKSIZE);
                }
            }
            assert(r == 0); // size must be a 16 byte block for Twofish to do it's job!
        }
        assert(cipher != nullptr); // can't process data without a twofish encryptor or decryptor!
    }

private:
    TwofishCrypt &operator=(const TwofishCrypt &rhs);
    TwofishCrypt(const TwofishCrypt &source);

protected:
    /** @brief assigned by a TwofishEncryptor or TwofishDecryptor */
    T *cipher;
};

//-----------------------------------------------------------------------

}// namespace Crypto

#endif	// _INCLUDED_TwofishCrypt_H
