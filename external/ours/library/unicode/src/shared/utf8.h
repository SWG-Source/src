#ifndef UTF8_H
#define UTF8_H

#include <stdio.h>
#include "Unicode.h"

namespace Unicode
{

typedef char UTF8;
typedef unsigned short int UTF16;

int UTF8_charSize( char * );

int UTF8_convertCharToUTF16( char * ptr , UTF16 * ret );
int UTF8_convertToUTF16( char * ptr , UTF16 * ret, int limit );
int UTF16_convertCharToUTF8( UTF16 ret, char * ptr , int limit );
int UTF16_convertToUTF8( UTF16 * ret, char * ptr , int limit );

void EnglishToFakeUtf8(char *fakeDestination, char *englishSource, int destinationSize);
void FakeUtf8ToEnglish(char *englishDestination, char *fakeSource, int destinationSize);

Unicode::String	UTF8ToUnicode(const char *source);
void					UnicodeToUTF8(Unicode::String &source, char *dest, int limit);

}; 

#endif

