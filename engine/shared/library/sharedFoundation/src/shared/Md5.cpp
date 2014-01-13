// ======================================================================
//
// Md5.cpp
//
// Portions copyright 2003 Sony Online Entertainment
// Portions derived from the RSA Data Security, Inc. MD5 Message-Digest Algorithm
// Portions derived from Dan Gorman's implementation
//
// ======================================================================

#include "sharedFoundation/FirstSharedFoundation.h"
#include "sharedFoundation/Md5.h"

#include <stdio.h>

// ======================================================================

Md5::Value::Value()
{
	memset(m_data, 0, sizeof(m_data));
}

// ----------------------------------------------------------------------

Md5::Value::Value(Value const &rhs)
{
	memcpy(m_data, rhs.m_data, sizeof(m_data));
}

// ----------------------------------------------------------------------

bool Md5::Value::operator ==(Value const &rhs) const
{
	return memcmp(m_data, rhs.m_data, sizeof(m_data)) == 0;
}

// ----------------------------------------------------------------------

bool Md5::Value::operator !=(Value const &rhs) const
{
	return !(*this == rhs);
}

// ----------------------------------------------------------------------

void Md5::Value::format(char *buffer, int bufferLength) const
{
	//not referenced in release build
	UNREF(bufferLength);
	DEBUG_FATAL(bufferLength < 33, ("PB: buffer must be at least 33 characters"));
	sprintf(buffer, "%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x", m_data[0], m_data[1], m_data[2], m_data[3], m_data[4], m_data[5], m_data[6], m_data[7], m_data[8], m_data[9], m_data[10], m_data[11], m_data[12], m_data[13], m_data[14], m_data[15]);
}

// ======================================================================

Md5::Value Md5::calculate(const char *string)
{
	return calculate(string, strlen(string));
}

// ----------------------------------------------------------------------

Md5::Value Md5::calculate(void const *data, int length)
{
	Md5 md5;
	md5.update(data, length);
	md5.finish();
	return md5.getValue();
}

// ======================================================================

Md5::Md5()
:
	m_bufferedBytes(0),
	m_totalLength(0),
	m_finished(false)
{
	reset();
}

// ----------------------------------------------------------------------

void Md5::reset()
{
  m_data[0] = 0x67452301;
  m_data[1] = 0xefcdab89;
  m_data[2] = 0x98badcfe;
  m_data[3] = 0x10325476;
	m_bufferedBytes = 0;
	m_totalLength = 0;
	m_finished = false;
}

// ----------------------------------------------------------------------

void Md5::update(void const *data, int length)
{
	m_totalLength += length;

	byte const * byteData = reinterpret_cast<byte const *>(data);
	while (length > 0)
	{
		// if there's no bytes pending from last time and we have a full block incoming transform it directly to avoid the copy
		if (m_bufferedBytes == 0 && length >= BUFFER_SIZE)
		{
			transformBlock(byteData);
			byteData += BUFFER_SIZE;
			length -= BUFFER_SIZE;
		}
		else
		{
			// figure out how many bytes to copy into the temp buffer
			int bytesToCopy = BUFFER_SIZE - m_bufferedBytes;
			if (length < bytesToCopy)
				bytesToCopy = length;

			// copy the data into the temp buffer
			memmove(m_buffer + m_bufferedBytes, byteData, bytesToCopy);
			m_bufferedBytes += bytesToCopy;
			byteData += bytesToCopy;
			length -= bytesToCopy;

			// if we've managed to fill the temp buffer, transform it
			if (m_bufferedBytes == BUFFER_SIZE)
			{
				transformBlock(m_buffer);
				m_bufferedBytes = 0;
			}
		}
	}
}

// ----------------------------------------------------------------------

void Md5::wordsToBytes(const uint32* words, size_t wordCnt, uint8* bytes)
{
	const int bytesPerWord = sizeof(*words)/sizeof(*bytes);
	while ((wordCnt--) > 0)
	{
		uint32 wd = *(words++);
		int cnt = bytesPerWord;
		while (cnt-- > 0)
		{
			*(bytes++) = static_cast<uint8>(wd & 0x0ff);
			wd >>= 8;
		}
	}
}

// ----------------------------------------------------------------------

void Md5::bytesToWords(const uint8* bytes, size_t byteCnt, uint32* words)
{
	const int bytesPerWord = sizeof(*words)/sizeof(*bytes);
	size_t wdcnt = byteCnt/bytesPerWord;
	while (wdcnt-- > 0)
	{
		uint32 wd = 0;
		int cnt = bytesPerWord;
		int shft = 0;
		while (cnt-- > 0)
		{
			wd |= static_cast<uint32>(*(bytes++) & 0x0ff) << shft;
			shft += 8;
		}
		*(words++) = wd;
		byteCnt -= bytesPerWord;
	}
}

// ----------------------------------------------------------------------

inline uint32 Md5::rotate_left(uint32 ival, uint32 shft)
{
	return ((ival << shft) | (ival >> (32-shft)));
}

// ----------------------------------------------------------------------

inline uint32 Md5::round1(uint32 i, uint32 j, uint32 k, uint32 l, uint32 i1, uint32 j1, uint32 k1)
{
	i += ((j & k) | (~j & l)) + i1 + k1;
	return rotate_left(i, j1) + j;
}

// ----------------------------------------------------------------------

inline uint32 Md5::round2(uint32 i, uint32 j, uint32 k, uint32 l, uint32 i1, uint32 j1, uint32 k1)
{
	i += ((j & l) | (k & ~l)) + i1 + k1;
	return rotate_left(i, j1) + j;
}

// ----------------------------------------------------------------------

inline uint32 Md5::round3(uint32 i, uint32 j, uint32 k, uint32 l, uint32 i1, uint32 j1, uint32 k1)
{
	i += (j ^ k ^ l) + i1 + k1;
	return rotate_left(i, j1) + j;
}

// ----------------------------------------------------------------------

inline uint32 Md5::round4(uint32 i, uint32 j, uint32 k, uint32 l, uint32 i1, uint32 j1, uint32 k1)
{
	i += (k ^ (j | ~l)) + i1 + k1;
	return rotate_left(i, j1) + j;
}

// ----------------------------------------------------------------------

void Md5::transformBlock(void const * data)
{
	// pack the bytes into 32-bit little endian words
	uint32 work[64 / 4];
	bytesToWords(reinterpret_cast<byte const *>(data), 64, work);
	//uint32 const * work = reinterpret_cast<uint32 const *>(data);

	uint32 j = m_data[0];
	uint32 k = m_data[1];
	uint32 l = m_data[2];
	uint32 m = m_data[3];

	j = round1(j, k, l, m, work[0],  7,  0xd76aa478);
	m = round1(m, j, k, l, work[1],  12, 0xe8c7b756);
	l = round1(l, m, j, k, work[2],  17, 0x242070db);
	k = round1(k, l, m, j, work[3],  22, 0xc1bdceee);
	j = round1(j, k, l, m, work[4],  7,  0xf57c0faf);
	m = round1(m, j, k, l, work[5],  12, 0x4787c62a);
	l = round1(l, m, j, k, work[6],  17, 0xa8304613);
	k = round1(k, l, m, j, work[7],  22, 0xfd469501);
	j = round1(j, k, l, m, work[8],  7,  0x698098d8);
	m = round1(m, j, k, l, work[9],  12, 0x8b44f7af);
	l = round1(l, m, j, k, work[10], 17, 0xffff5bb1);
	k = round1(k, l, m, j, work[11], 22, 0x895cd7be);
	j = round1(j, k, l, m, work[12], 7,  0x6b901122);
	m = round1(m, j, k, l, work[13], 12, 0xfd987193);
	l = round1(l, m, j, k, work[14], 17, 0xa679438e);
	k = round1(k, l, m, j, work[15], 22, 0x49b40821);

	j = round2(j, k, l, m, work[1],  5,  0xf61e2562);
	m = round2(m, j, k, l, work[6],  9,  0xc040b340);
	l = round2(l, m, j, k, work[11], 14, 0x265e5a51);
	k = round2(k, l, m, j, work[0],  20, 0xe9b6c7aa);
	j = round2(j, k, l, m, work[5],  5,  0xd62f105d);
	m = round2(m, j, k, l, work[10], 9,  0x02441453);
	l = round2(l, m, j, k, work[15], 14, 0xd8a1e681);
	k = round2(k, l, m, j, work[4],  20, 0xe7d3fbc8);
	j = round2(j, k, l, m, work[9],  5,  0x21e1cde6);
	m = round2(m, j, k, l, work[14], 9,  0xc33707d6);
	l = round2(l, m, j, k, work[3],  14, 0xf4d50d87);
	k = round2(k, l, m, j, work[8],  20, 0x455a14ed);
	j = round2(j, k, l, m, work[13], 5,  0xa9e3e905);
	m = round2(m, j, k, l, work[2],  9,  0xfcefa3f8);
	l = round2(l, m, j, k, work[7],  14, 0x676f02d9);
	k = round2(k, l, m, j, work[12], 20, 0x8d2a4c8a);

	j = round3(j, k, l, m, work[5],  4,  0xfffa3942);
	m = round3(m, j, k, l, work[8],  11, 0x8771f681);
	l = round3(l, m, j, k, work[11], 16, 0x6d9d6122);
	k = round3(k, l, m, j, work[14], 23, 0xfde5380c);
	j = round3(j, k, l, m, work[1],  4,  0xa4beea44);
	m = round3(m, j, k, l, work[4],  11, 0x4bdecfa9);
	l = round3(l, m, j, k, work[7],  16, 0xf6bb4b60);
	k = round3(k, l, m, j, work[10], 23, 0xbebfbc70);
	j = round3(j, k, l, m, work[13], 4,  0x289b7ec6);
	m = round3(m, j, k, l, work[0],  11, 0xeaa127fa);
	l = round3(l, m, j, k, work[3],  16, 0xd4ef3085);
	k = round3(k, l, m, j, work[6],  23, 0x04881d05);
	j = round3(j, k, l, m, work[9],  4,  0xd9d4d039);
	m = round3(m, j, k, l, work[12], 11, 0xe6db99e5);
	l = round3(l, m, j, k, work[15], 16, 0x1fa27cf8);
	k = round3(k, l, m, j, work[2],  23, 0xc4ac5665);

	j = round4(j, k, l, m, work[0],  6,  0xf4292244);
	m = round4(m, j, k, l, work[7],  10, 0x432aff97);
	l = round4(l, m, j, k, work[14], 15, 0xab9423a7);
	k = round4(k, l, m, j, work[5],  21, 0xfc93a039);
	j = round4(j, k, l, m, work[12], 6,  0x655b59c3);
	m = round4(m, j, k, l, work[3],  10, 0x8f0ccc92);
	l = round4(l, m, j, k, work[10], 15, 0xffeff47d);
	k = round4(k, l, m, j, work[1],  21, 0x85845dd1);
	j = round4(j, k, l, m, work[8],  6,  0x6fa87e4f);
	m = round4(m, j, k, l, work[15], 10, 0xfe2ce6e0);
	l = round4(l, m, j, k, work[6],  15, 0xa3014314);
	k = round4(k, l, m, j, work[13], 21, 0x4e0811a1);
	j = round4(j, k, l, m, work[4],  6,  0xf7537e82);
	m = round4(m, j, k, l, work[11], 10, 0xbd3af235);
	l = round4(l, m, j, k, work[2],  15, 0x2ad7d2bb);
	k = round4(k, l, m, j, work[9],  21, 0xeb86d391);

	m_data[0] += j;
	m_data[1] += k;
	m_data[2] += l;
	m_data[3] += m;
}

// ----------------------------------------------------------------------

void Md5::finish()
{
	DEBUG_FATAL(m_finished, ("PB: MD5 already finished"));

	uint64 messageLengthInBits = m_totalLength * 8;
	
	// we always have to append a trailing bit
	{
		byte append = 0x80;
		update(&append, 1);
	}

	// how much to pad to existing bytes to come out to an even 64 byte multiple, yet leave room at the end for the count.
	while (m_bufferedBytes != (BUFFER_SIZE - 8))
	{
		byte append = 0x00;
		update(&append, 1);
	}

	// append the length of the source message in bits
	uint8 countAsBytes[8];
	wordsToBytes(reinterpret_cast<const uint32*>(&(messageLengthInBits)), 2, countAsBytes);
	update(countAsBytes, 8);

	m_finished = true;
}

// ----------------------------------------------------------------------

Md5::Value Md5::getValue() const
{
	DEBUG_FATAL(!m_finished, ("PB: did not finish MD5 before calling getValue"));

	Value v;
	wordsToBytes(m_data, 4, v.m_data);
	return v;
}

// ======================================================================
