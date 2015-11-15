#ifndef BASE64_H
#define BASE64_H

#include <string>

#ifdef EXTERNAL_DISTRO
namespace NAMESPACE 
{

#endif
namespace Base 
{
	class Base64
	{
	public:
		static bool UUEncode(const unsigned char *source, unsigned sourceLen, char *dest, unsigned destLen);
		static bool UUEncode(const unsigned char *source, unsigned sourceLen, std::string &destString);
		static bool UUDecode(const char *source, unsigned sourceLen, unsigned char *dest, int *destLen);
		static bool UUDecode(const char *source, unsigned sourceLen, std::string &destString);

		// these don't *exactly* belong here, but it's a convenient place for them
		static bool HexEncode(const unsigned char *source, unsigned sourceLen, char *dest, unsigned destLen, bool lowercase = false);
		static bool HexEncode(const unsigned char *source, unsigned sourceLen, std::string &destString, bool lowercase = false);
		static bool HexDecode(const char *source, unsigned sourceLen, unsigned char *dest, unsigned *destLen);
		static bool HexDecode(const char *source, unsigned sourceLen, std::string &destString);

	private:
		static unsigned UUEncodeTripleToQuad(const unsigned char *triple, char *quad, unsigned bytes);
		static unsigned UUDecodeQuadToTriple(const char *quad, unsigned char *triple);

		static void Init();
		static unsigned char GetBase64Code(char base64char);
		static unsigned char GetHexCode(char hexChar);

		static const char ms_base64Table[];
	};
};
#ifdef EXTERNAL_DISTRO
};
#endif

#endif  //  BASE64_H
