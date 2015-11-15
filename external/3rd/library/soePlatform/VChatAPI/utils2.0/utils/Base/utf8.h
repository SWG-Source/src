#ifndef UTF8_H
#define UTF8_H

#include <stdio.h>
//#include "Unicode.h"

namespace soe
{

typedef char UTF8;
typedef unsigned short int UTF16;

size_t UTF8_charSize( char * );

size_t UTF8_convertCharToUTF16( char * ptr , UTF16 * ret );
size_t UTF8_convertToUTF16( char * ptr , UTF16 * ret, size_t limit );
size_t UTF16_convertCharToUTF8( UTF16 ret, char * ptr , size_t limit );
size_t UTF16_convertToUTF8( UTF16 * ret, char * ptr , size_t limit );

void EnglishToFakeUtf8(char *fakeDestination, char *englishSource, size_t destinationSize);
void FakeUtf8ToEnglish(char *englishDestination, char *fakeSource, size_t destinationSize);

//Plat_Unicode::String	UTF8ToUnicode(const char *source);
//void					UnicodeToUTF8(Plat_Unicode::String &source, char *dest, int limit);
size_t UTF16_strlen( UTF16 * ptr );
int UTF8_strlen( const char * );

//truncate UTF-8
int UTF8_truncate( char *, const char*, size_t limit);

};
#endif

