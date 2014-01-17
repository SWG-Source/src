#ifndef API__HASH_H
#define API__HASH_H

#include <stdlib.h>
#include <ctype.h>
#include <time.h>
#include <assert.h>
#include <string>
#include <string.h>
#include "types.h"
#include "Base/stringutils.h"
#include <stdio.h>

#ifdef EXTERNAL_DISTRO
namespace NAMESPACE 
{
#endif

    #define MAX_HASH_VALUE          0xfe
    #define BROADCAST_HASH_VALUE    0xff

    inline unsigned char HashAccountId(unsigned accountId) { return (accountId & MAX_HASH_VALUE); }

    inline unsigned char HashUserName(const char * name)
    {
	    unsigned value = 0;
	    while (*name)
	    {
		    value += tolower(*name);
		    name++;
	    }
	    return value % MAX_HASH_VALUE;
    }

    inline unsigned char HashRandom() {  return (rand() % MAX_HASH_VALUE);  }

const soe::uint64 nMaxCharBits = 0x3f;
const unsigned char nMaxBits = 64;
const unsigned char nEncodeCharBits = 6;
const unsigned char nMaxHashChar = 2;
const unsigned char nMaxRandomChar = 3;
const unsigned nEncodeChars = nMaxBits/nEncodeCharBits;
const unsigned nMaxEncodeChars = nEncodeChars + nMaxRandomChar+ nMaxHashChar + 1;

static const char base64[65] = "0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ.?";

unsigned CalcMaxNumforBits(unsigned nBits);
char DecodeChar(char c);
//int HexStringToInt(const char *value);
int HexStringToInt(const std::string &value);

#ifdef _DEBUG
inline std::string printBase64()
{
	std::string tmp;
	for (unsigned i = 0; i < nMaxBits; i++)
	{
		char buf[16];
		sprintf(buf, "%02d %c=%d, %d\n", i, base64[i], base64[i], DecodeChar(base64[i]));
		tmp += buf;
	}
	return tmp;
}
#endif

/////////////////////////////////////////////////////////////////////////////
// Session ID is a 16 byte string constructed with account ID, random number, 
//	and hash value. Here is the detail:
//		Account ID		= 11 bytes (base64 encoded)
//		Random Number	=  3 bytes (base64 encoded)
//		Hash Value		=  2 bytes (Hex encoded)
//

inline std::string EncodeSessionID(soe::uint64 accountID, unsigned char hash)
{
	char encodeBuf[64];
	memset(encodeBuf,0,64);

	for (unsigned i = 0; i < nEncodeChars; i++)
	{
		soe::uint64 shift = nMaxBits-(nEncodeCharBits*(i+1));
		soe::uint64 tmp = accountID >> shift;
		unsigned char ch = (unsigned char)(tmp & nMaxCharBits);
		encodeBuf[i] = base64[ch];
	}

	unsigned char remainingBits = nMaxBits % nEncodeCharBits;
	if (remainingBits != 0)
	{
		unsigned char nMaxRemainingBits = CalcMaxNumforBits(remainingBits);
		unsigned char tmp = (unsigned char)(accountID & nMaxRemainingBits);
		encodeBuf[nEncodeChars] = base64[tmp];
	}

	char randomNum[4];
	randomNum[0] = base64[rand()%nMaxBits];
	randomNum[1] = base64[rand()%nMaxBits];
	randomNum[2] = base64[rand()%nMaxBits];
	randomNum[3] = 0;

	// add hash
	char hashBuf[8];
	sprintf(hashBuf, "%02x", hash);
	std::string tmp = std::string(encodeBuf) + std::string(randomNum) + std::string(hashBuf);

	return tmp; 
}


inline bool DecodeSessionID(const std::string & sessionID, soe::uint64 & accountID, unsigned char & hash)
{
	if (sessionID.empty() || sessionID.size() > nMaxEncodeChars)
		return false;

	accountID = 0;
	std::string sDecode = sessionID.substr(0, sessionID.size()-nMaxHashChar-nMaxRandomChar);
	std::string sHash = sessionID.substr(sessionID.size()-nMaxHashChar, nMaxHashChar);

	hash = (unsigned char)HexStringToInt(sHash);

	for (unsigned i = 0; i < nEncodeChars; i++)
	{
		char c = DecodeChar(sDecode[i]);
		if (c == -1)
		{
			// invalid character
			return false;
		}

		soe::uint64 decodeNum = (soe::uint64)c;
		if (i == 0)
		{
			accountID = decodeNum;
		}
		else
		{
			accountID = (accountID << nEncodeCharBits) | decodeNum;
		}
	}

	unsigned char remainingBits = nMaxBits % nEncodeCharBits;
	if (remainingBits != 0)
	{
		soe::uint64 decodeNum = (soe::uint64)DecodeChar(sDecode[sDecode.size()-1]);
		accountID = (accountID << remainingBits) | decodeNum;
	}

	return true;
}

inline unsigned CalcMaxNumforBits(unsigned nBits)
{
	unsigned maxNum = 0;
	for (unsigned i = 0; i < nBits; i++)
	{
		maxNum |= 1 << i; // pow(2,i);
	}
	return maxNum;
}

inline char DecodeChar(char c)
{
	char num = -1;
	if (c >= 48 && c <= 57)
	{
		num = c - 48;
	}
	else if (c >= 97 && c <= 122)
	{
		num = c - 87;
	}
	else if (c >= 65 && c <= 90)
	{
		num = c - 29;
	}
	else if (c == 46)
	{
		num = 62;
	}
	else if (c == 63)
	{
		num = 63;
	}
	
	return num;
}

inline int HexStringToInt(const std::string &value)
{
  struct CHexMap
  {
    char chr;
    int value;
  };
  const int HexMapL = 16;
  CHexMap HexMap[HexMapL] =
  {
    {'0', 0}, {'1', 1},
    {'2', 2}, {'3', 3},
    {'4', 4}, {'5', 5},
    {'6', 6}, {'7', 7},
    {'8', 8}, {'9', 9},
    {'A', 10}, {'B', 11},
    {'C', 12}, {'D', 13},
    {'E', 14}, {'F', 15}
  };

  //char *mstr = strupr(strdup(value));
  std::string upperstr = soe::touppercase(value);
  const char *s = upperstr.c_str();
  int result = 0;
  if (*s == '0' && *(s + 1) == 'X') s += 2;
  bool firsttime = true;
  while (*s != '\0')
  {
    bool found = false;
    for (int i = 0; i < HexMapL; i++)
    {
      if (*s == HexMap[i].chr)
      {
        if (!firsttime) result <<= 4;
        result |= HexMap[i].value;
        found = true;
        break;
      }
    }
    if (!found) break;
    s++;
    firsttime = false;
  }
  //free(mstr);
  return result;
}

#ifdef EXTERNAL_DISTRO
};
#endif

#endif
