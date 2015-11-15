#include "Base64.h"

#include <cstdio>
#include <cstring>

#ifdef EXTERNAL_DISTRO
namespace NAMESPACE 
{

#endif
namespace Base 
{

	//----------------------------------------------------------------------------
	// Constants
	//----------------------------------------------------------------------------
	#define BASE64_PAD '='
	#define BASE64_PAD_INDEX 64
	#define BASE64_ERR ((unsigned char)-1)
	#define BASE64_PAD_CODE ((unsigned char)100)
	#define BASE64_OUTPUT_LINE 76
	#define BASE64_ENCODE_BUFFER ((BASE64_OUTPUT_LINE*3)/4)
	#define BASE64_DECODE_ERROR	((unsigned)-1)

	#define HEX_ERR 16
	#define HEX_LOWER_FORMAT "%2.2x"
	#define HEX_UPPER_FORMAT "%2.2X";

	const char Base64::ms_base64Table[] =
	{
		'A',
		'B',
		'C',
		'D',
		'E',
		'F',
		'G',
		'H',
		'I',
		'J',
		'K',
		'L',
		'M',
		'N',
		'O',
		'P',
		'Q',
		'R',
		'S',
		'T',
		'U',
		'V',
		'W',
		'X',
		'Y',
		'Z',
		'a',
		'b',
		'c',
		'd',
		'e',
		'f',
		'g',
		'h',
		'i',
		'j',
		'k',
		'l',
		'm',
		'n',
		'o',
		'p',
		'q',
		'r',
		's',
		't',
		'u',
		'v',
		'w',
		'x',
		'y',
		'z',
		'0',
		'1',
		'2',
		'3',
		'4',
		'5',
		'6',
		'7',
		'8',
		'9',
		'+',
		'/',
		BASE64_PAD
	};


	//----------------------------------------------------------------------------
	// Functions
	//----------------------------------------------------------------------------
	
	unsigned char Base64::GetBase64Code(char base64char)
	{
		if (base64char == BASE64_PAD)
			return BASE64_PAD_CODE;
		
		if (base64char >= 'A' && base64char <= 'Z')
			return (unsigned char)(base64char - 'A');
		if (base64char >= 'a' && base64char <= 'z')
			return (unsigned char)(base64char - 'a' + 26);
		if (base64char >= '0' && base64char <= '9')
			return (unsigned char)(base64char - '0' + 2*26);
		if (base64char == '+')
			return 2*26+10;
		if (base64char == '/')
			return 2*26+11;

		return BASE64_ERR;
	}

	unsigned char Base64::GetHexCode(char hexChar)
	{
		if (hexChar >= '0' && hexChar <= '9')
			return (unsigned char)(hexChar - '0');
		if (hexChar >= 'A' && hexChar <= 'F')
			return (unsigned char)(hexChar - 'A' + 10);
		if (hexChar >= 'a' && hexChar <= 'f')
			return (unsigned char)(hexChar - 'a' + 10);

		return HEX_ERR;
	}

	unsigned Base64::UUDecodeQuadToTriple(const char *quad, unsigned char *triple)
	{
		unsigned char b;
		unsigned char quadByteCodes[4];
		unsigned bytes = 3, i;

		for (i = 0; i < 4; i++)
		{
			b = GetBase64Code(quad[i]);

			if (b == BASE64_PAD_CODE)
			{
				switch(i)
				{
					case 0: bytes=0; break;
					case 1: bytes=0; break;
					case 2: bytes=1; break;
					case 3: bytes=2; break;
				}
				break;
			}
			else if (b == BASE64_ERR)
			{
				bytes = BASE64_DECODE_ERROR;
			}
			else
			{
				quadByteCodes[i] = b;
			}
			
		}

		triple[0] =	(quadByteCodes[0] << 2) | (quadByteCodes[1] >> 4);
		triple[1] = (quadByteCodes[1] << 4) | (quadByteCodes[2] >> 2);
		triple[2] = (quadByteCodes[2] << 6) | (quadByteCodes[3] >> 0);

		return bytes;
	}

	bool Base64::UUDecode(const char *source, unsigned sourceLen, unsigned char *dest, int *destLen)
	{
		unsigned char q[3];
		unsigned index;
		unsigned bytes;
		unsigned char *destIn = dest;
		bool noErrors = true;

		for (index = 0; index < sourceLen; index += 4)
		{
			bytes = UUDecodeQuadToTriple(source + index, q);

			if (bytes != BASE64_DECODE_ERROR) {
				*destLen -= bytes;
				if (*destLen >= 0) {
					memcpy(dest, (char*)&q, bytes);
				} else {
					noErrors = false;
				}
				dest += bytes;
			} else {
				noErrors = false;
			}
		}
		*destLen = (int)(dest - destIn);

		return noErrors;
	}

	bool Base64::UUDecode(const char *source, unsigned sourceLen, std::string &destString)
	{
		unsigned char q[3];
		unsigned index;
		unsigned bytes;
		bool noErrors = true;

		for (index = 0; index < sourceLen; index += 4)
		{
			bytes = UUDecodeQuadToTriple(source + index, q);

			if (bytes != BASE64_DECODE_ERROR) {
				destString.append((char*)&q, bytes);
			} else {
				noErrors = false;
			}
		}

		return noErrors;
	}

	unsigned Base64::UUEncodeTripleToQuad(const unsigned char *triple, char* quad, unsigned bytes)
	{
		unsigned char indices[4];
		int quadIndex;

		memset(indices, BASE64_PAD_INDEX, sizeof(indices));

		switch(bytes)
		{
		case 3:
			indices[3] =			   (triple[2]     ) & 0x0000003F;
			indices[2] =			   (triple[2] >> 6) & 0x0000003F;
		case 2:
			indices[2] = (indices[2] | (triple[1] << 2)) & 0x0000003F;
			indices[1] =			   (triple[1] >> 4) & 0x0000003F;
		case 1:
			indices[1] = (indices[1] | (triple[0] << 4)) & 0x0000003F;
			indices[0] =			   (triple[0] >> 2)  & 0x0000003F;
		case 0:
		default:
			break;
		};

		for (quadIndex = 0; quadIndex < 4; quadIndex++) {
			quad[quadIndex] = ms_base64Table[indices[quadIndex]];
		}

		return 0;
	}

	bool Base64::UUEncode(const unsigned char *source, unsigned sourceLen, char *dest, unsigned destLen)
	{
		char quad[5];
		unsigned z;
		bool noErrors = true;

		quad[4] = 0;
	    
		// The normal case
		for(z = 0; z < sourceLen;z += 3)
		{
			UUEncodeTripleToQuad(source + z,quad,(z + 3 < sourceLen) ? 3 : (sourceLen - z));
			destLen -= strlen(quad);
			if (destLen > 0) {
				dest += sprintf(dest,quad);
			} else {
				noErrors = false;
			}
		}

		return noErrors;
	}

	bool Base64::UUEncode(const unsigned char *source, unsigned sourceLen, std::string &destString)
	{
		char quad[5];
		unsigned z;
		bool noErrors = true;

		quad[4] = 0;
	    
		// The normal case
		for(z = 0; z < sourceLen;z += 3)
		{
			UUEncodeTripleToQuad(source + z,quad,(z + 3 < sourceLen) ? 3 : (sourceLen - z));

			destString.append(quad);
		}

		return noErrors;
	}

	bool Base64::HexEncode(const unsigned char *source, unsigned sourceLen, char *dest, unsigned destLen, bool lowercase)
	{
		const char *format = lowercase ? HEX_LOWER_FORMAT : HEX_UPPER_FORMAT;
		bool noErrors = true;

		for (; sourceLen > 0; sourceLen--, source++)
		{
			if (destLen >= 2) {
				destLen -= sprintf(dest, format, *source);
			} else {
				noErrors = false;
			}
		}

		return noErrors;
	}

	bool Base64::HexEncode(const unsigned char *source, unsigned sourceLen, std::string &destString, bool lowercase)
	{
		const char *format = lowercase ? HEX_LOWER_FORMAT : HEX_UPPER_FORMAT;
		char hexBuffer[3];
		bool noErrors = true;

		hexBuffer[2] = '\0';

		for (; sourceLen > 0; sourceLen--, source++)
		{
			hexBuffer[0] = '\0';
			sprintf(hexBuffer, format, *source);
			destString.append(hexBuffer);
		}

		return noErrors;
	}

	bool Base64::HexDecode(const char *source, unsigned sourceLen, unsigned char *dest, unsigned *destLen)
	{
		unsigned char *destIn = dest;
		unsigned char digits[2];
		bool noErrors = true;

		for (; sourceLen > 1; sourceLen -= 2, source += 2)
		{
			if (*destLen > 0) {
				digits[0] = GetHexCode(source[0]);
				digits[1] = GetHexCode(source[1]);
				if ((digits[0] != HEX_ERR) && (digits[1] != HEX_ERR)) {
					*dest = (digits[0] * 16) + digits[1];
					dest++;
					(*destLen)--;
				} else {
					noErrors = false;
				}
			} else {
				noErrors = false;
			}
		}
		*destLen = dest - destIn;

		if (sourceLen == 1) {
			noErrors = false;
		}

		return noErrors;
	}

	bool Base64::HexDecode(const char *source, unsigned sourceLen, std::string &destString)
	{
		unsigned char digits[2];
		unsigned char byte;
		bool noErrors = true;

		for (; sourceLen > 1; sourceLen -= 2, source += 2)
		{
			digits[0] = GetHexCode(source[0]);
			digits[1] = GetHexCode(source[1]);
			if ((digits[0] != HEX_ERR) && (digits[1] != HEX_ERR)) {
				byte = (digits[0] * 16) + digits[1];
				destString.append((char *)&byte, 1);
			} else {
				noErrors = false;
			}
		}

		if (sourceLen == 1) {
			noErrors = false;
		}

		return noErrors;
	}

};
#ifdef EXTERNAL_DISTRO
};
#endif
