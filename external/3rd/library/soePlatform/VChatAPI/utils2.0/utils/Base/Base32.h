#if !defined(_BASE32_H_)
#define _BASE32_H_


#ifdef EXTERNAL_DISTRO
namespace NAMESPACE 
{
#endif

	namespace Base 
	{
		struct Base32
		{
			// Decode32 input is the output of Encode32. The out parameters should be unsigned char[] of
			// length GetDecode32Length(inLen) and GetEncode32Length(inLen) respectively.
			static bool Decode32(unsigned char* in, int inLen, unsigned char* out);

			// Encode32 outputs at out bytes with values from 0 to 32 that can be mapped to 32 signs.
			static bool Encode32(unsigned char* in, int inLen, unsigned char* out);

			static int  GetDecode32Length(int bytes);
			static int  GetEncode32Length(int bytes);

			// Both Map32 and Unmap32 do inplace modification of the inout32 array.
			// The alpha32 array must be exactly 32 chars long.
			// To map the output of Encode32 to an alphabet of 32 characters use Map32.
			static bool Map32(unsigned char* inout32, int inout32Len, unsigned char* alpha32);

			// To unmap back the output of Map32 to an array understood by Decode32 use Unmap32.
			static bool Unmap32(unsigned char* inout32, int inout32Len, unsigned char* alpha32);

			// same as above but user the static map, ms_base32Table
			static bool Map32(unsigned char* inout32, int inout32Len);
			static bool Unmap32(unsigned char* inout32, int inout32Len);

			static const unsigned char ms_base32Table[];
		};
	};

#ifdef EXTERNAL_DISTRO
};
#endif

#endif // _BASE32_H_
