#include <string.h>
#include "utf8.h"

namespace soe
{

    /*
    A little bit about UTF8...

    UTF8 stands for UCS Tranfer Format, while UCS stands for Universal Character Set.  In
    essence, UTF8 is a way to encode all the characters in the UCS.  UTF8 attempts to
    describe all the characters in the UCS using from 1 to 6 bytes per character.  If the
    first byte's high bit is not set, then the character is 1 byte long, and corresponds to
    the US-ASCII character of the same number.  In this way, all current US-ASCII strings are
    already UTF8 compatible.  If the the first byte's high bit is set, then the number of
    high bits set until the first 0 describes the number of bytes in the character, e.g. a
    first byte who's bits are 1110xxxx will be followed by 2 more "data bytes" that look like
    10yyyyyy 10zzzzzz, so that the entire character's descriptive ID is x xxxxyyyy yyzzzzzz.

    With this method, all characters in the UCS-4 character space can be mapped to <=6 bytes of
    data, and all characters int the UCS-2 character space can be mapped in <=3 bytes
    maximum.  Since only UCS-2 actually contains character assignments from known languages,
    it is expected that a maximum of 3 bytes will be needed for any character encountered
    during translation.

    Unfortunately, this method also means that there are sequences of bytes that do not form
    valid UTF8 characters/strings.

    In this file, we've placed some string and character validation functions, as well as
    replacements for functions where the standard <string.h> function will not behave
    properly on a UTF8 string.

    */

    // array of character byte-lengths.  Index into the array with the first byte of the character & 0x3C then shift down twice.
    // NOTE: this only works for multi-byte characters (up to 6).
    static char UTF8_charSizeArray[16] = { 2, 2, 2, 2, 2, 2, 2, 2, 3, 3, 3, 3, 4, 4, 5, 6 };


    // UTF8_charSize returns the length of a UTF8 character in bytes (1-6).
    // The array UTF8_size has 0's where an invalid header byte is detected.
    // This is a "fast" function, so no error checking is done.
    size_t UTF8_charSize( char * ptr )
    {
        // temp hold of "header" byte
        unsigned char ct = *ptr;
        // if high bit not set, it's just a normal character, length 1
        if (( ct & 0x80 ) == 0 )
        {
            return 1;
        }
        return (size_t)UTF8_charSizeArray[ ( ct & 0x3C ) >> 2 ];
    }



    // takes a pointer to a UTF8 character and fills the 2nd arg with the 16-bit UTF16 corresponding char.
    // return value is the length of the UTF8 char converted.
    size_t UTF8_convertCharToUTF16( char * ptr , UTF16 * ret )
    {
        size_t len = UTF8_charSize( ptr );
        if ( len == 1 )
        {
            *ret = *ptr;
        }
        else if ( len == 2 )
        {
            *ret = (UTF16)(((UTF16)( *ptr & 0x1F ) << 6 ) + ( *(ptr+1) & 0x3F ));
        }
        else if ( len == 3 )
        {
            *ret = (UTF16)(((UTF16)( *ptr & 0x0F ) << 12 ) + ((UTF16)( *(ptr+1) & 0x3F ) << 6 ) + (( *(ptr+2) & 0x3F )));
        }
        return len;
    }


    // takes a pointer to a UTF8 String and fills the 2nd array with the corresponding UTF16 string, up to
    // the limit number of characters.
    // return value is the length of the UTF16 string converted.
    size_t UTF8_convertToUTF16( char * ptr , UTF16 * ret, size_t limit )
    {
	    size_t len = 0;
        while (( *ptr != 0 ) && ( len < ( limit - 1 ) ))//we use limit -1 to leave room for the nullptr terminator
        {
            size_t clen = UTF8_convertCharToUTF16( ptr, ret );
            ptr += clen;
            ret++;
            len++;
        }
        *ret = 0;
        return len;
    }


    // takes a UTF16 character and fills the 2nd arg with the UTF8 corresponding char.
    // return value is the size of the UTF8 char converted.
    // return value is 0 if the size of the UTF8 char would be bigger than the limit allowed.  In this
    // case, the return string is not filled.
    size_t UTF16_convertCharToUTF8( UTF16 char16, char * ret, size_t limit )
    {
        size_t len = 0;
        if ( char16 <= 0x0080 )
        {
            if ( limit >= 1 )
            {
                *ret = (char)char16;
                len = 1;
            }
        }
        else if ( char16 <= 0x07FF )
        {
            if ( limit >= 2 )
            {
                *(ret) = (char)( 0xC0 + (( char16 & 0x07C0 ) >> 6 ));
                *(ret+1) = (char)(0x80 + (char)( char16 & 0x003F ));
                len = 2;
            }
        }
        else
        {
            if ( limit >= 3 )
            {
                *ret = (char)(0xE0 + (( char16 & 0xF000 ) >> 12 ));
                *(ret+1) = (char)(0x80 + (( char16 & 0x0FC0 ) >> 6 ));
                *(ret+2) = (char)(0x80 + ( char16 & 0x003F ));
                len = 3;
            }
        }
        return len;
    }

    // takes a pointer to a UTF16 String and fills the 2nd array with the corresponding UTF8 string, up to
    // the limit number of bytes.
    // return value is the size of the UTF8 string converted.
    size_t UTF16_convertToUTF8( UTF16 *ptr, char * ret, size_t limit )
    {
        size_t len = 0;
        while (( *ptr != 0 ) && ( limit != 0 ))
        {
            size_t clen = UTF16_convertCharToUTF8( *ptr, ret, limit );
            if ( clen == 0 )
            {
                break;
            }
            ret += clen;
            ptr++;
            limit -= clen;
        }
        *ret = 0;
        return len;
    }


    void EnglishToFakeUtf8(char *fakeDestination, char *englishSource, size_t destinationSize)
    {
	    UTF16 utf16[8192];

	    UTF16 *d = utf16;
	    char *s = englishSource;
	    bool afterPercent = false;
	    while (*s != 0)
	    {
		    if (afterPercent)
		    {
			    *d = *s;

			    if (*s == ' ')			// assuming everthing between % and white-space is formatting and don't shift it, this may not be the case all the time, but the worst that will happen is we won't shift everything we possibly could have
				    afterPercent = false;
		    }
		    else
		    {
			    if ((*s >= 'a' && *s <= 'k') || (*s >= 'A' && *s <= 'Z'))
				    *d = (UTF16)(*s + 0xfee0);
			    else
				    *d = *s;

			    if (*s == '%')		// we can't be shifting formatting tokens or else sprintf won't work
				    afterPercent = true;
		    }
		    d++;
		    s++;
	    }
	    *d = 0;

	    UTF16_convertToUTF8(utf16, fakeDestination, destinationSize);
    }

    void FakeUtf8ToEnglish(char *englishDestination, char *fakeSource, size_t destinationSize)
    {
	    UTF16 utf16[8192];
	    UTF8_convertToUTF16(fakeSource, utf16, sizeof(utf16));

	    UTF16 *s = utf16;
	    char *d = englishDestination;
	    char *endDest = d + destinationSize - 1;
	    while (*s != 0 && d < endDest)
	    {
		    if (*s >= 0xfee0)
			    *d = (char)(*s - 0xfee0);
		    else
			    *d = (char)*s;
		    d++;
		    s++;
	    }
	    *d = 0;
    }

    size_t UTF16_strlen( UTF16 * ptr )
    {
	    size_t count(0);
	    while(*ptr != 0)
	    {
		    ptr++;
		    count++;
	    }
	    return count;
    }

    static const int utf8_lengths[16]=
    {
        1,1,1,1,1,1,1,1,        // 0000 to 0111 : 1 byte (plain ASCII)
        0,0,0,0,                // 1000 to 1011 : not valid
        2,2,                    // 1100, 1101 : 2 bytes
        3,                      // 1110 : 3 bytes
        4                       // 1111 :4 bytes 
    };

    int UTF8_strlen(const char * source)
    {
        int wchar_length=0; 
        int code_size;		
        unsigned char byte;
        if (source)
        {
            int byte_length = (int)strlen(source);
            while (byte_length > 0)
            {
                byte=(unsigned char)*source;
                // Use lookup table to determine sequence 
                // length based on upper 4 bits of first byte 
                if ((byte <= 0xF7) && (0 != (code_size=utf8_lengths[ byte >> 4])))
                {
                    // 1 sequence == 1 character
                    wchar_length++;
                    if (code_size==4)
                    {
                        wchar_length++;
                    }
                    source+=code_size;        // increment pointer
                    byte_length-=code_size;   // decrement counter
                }
                else
                {
                    // invalid character encountered
                    return -1;
                }
            }
        }
        return wchar_length;
    }

    int UTF8_truncate(char *output, const char *source, size_t limit)
    {
        int wchar_length=0; //number of utf-8 characters
        int code_size;		//number of bytes each wchar
	unsigned cur_bytes_count = 0;
        unsigned char byte;
		char *pout = output;

		if(limit <= 0) 
			return -1;

        if (source)
        {
            int byte_length = (int)strlen(source);
            while (byte_length > 0)
            {
                byte=(unsigned char)*source;
                // Use lookup table to determine sequence 
                // length based on upper 4 bits of first byte 
                if ((byte <= 0xF7) && (0 != (code_size=utf8_lengths[ byte >> 4])))
                {
                    // 1 sequence == 1 character
					cur_bytes_count+=code_size;

					if(cur_bytes_count <= limit)
					{
						strncpy(pout, source, code_size);
					} else {
						break;
					}

                    wchar_length++;
                    if (code_size==4)
                    {
                        wchar_length++;
                    }
                    source+=code_size;        // increment pointer
					pout+=code_size;
                    byte_length-=code_size;   // decrement counter
                }
                else
                {
                    // invalid character encountered
                    return -1;
                }
            }
        }
        return wchar_length;
    }
};
