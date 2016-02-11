#include "Config.h"

//#include <stdlib.h>
//#include <stdio.h>
//#include <string.h>
#include <ctype.h>

#include "Platform.h"

#ifdef EXTERNAL_DISTRO
namespace NAMESPACE 
{

#endif
namespace Base 
{

//-----------------------------------
/// Read the config file into memory
/// Returns true for success
bool CConfig::LoadFile(char * file)
//-----------------------------------
{
	char ch;
    FILE * fp;

    UnloadFile();
	
    // open file
	fp = fopen(file, "r");
	if (fp == nullptr || fp == (FILE *)-1)
	{
		//fprintf(stderr,"Failed to open config file %s!",file);
		delete fp;
		return false;
	}

    // get length of file
	fseek(fp,0,SEEK_END);
	long length = ftell(fp);
	rewind(fp);

    // allocate buffer
	pConfig = new char[length + 1];

	memset(pConfig,0,length);

    // read data, stripping comments
	int i = 0;
	while (i < length)
	{
		ch = fgetc(fp);
		if (feof(fp))
            break;
        if (ch == '#')
		{
			while (ch != 10 && !feof (fp))
			{
				ch = fgetc(fp);
			}
		}
		pConfig[i++] = ch;
	}
    pConfig[i] = 0; // mark end of file buffer

	fclose(fp);
    return true;
}

//-----------------------------------
/// remove the config file from memory
void CConfig::UnloadFile(void)
//-----------------------------------
{
	delete[] pConfig;
	pConfig = nullptr;
}

//-----------------------------------
/// finds a key of the config in memory
// key argument is case-insensitive, but must be upper case in config file
// Returns true for success (passing nullptr is a special case returned as success)
bool CConfig::FindKey(char *key)
//-----------------------------------
{
	if (pConfig == nullptr)
		return false;

    // special case...continue with existing key
    if (key == nullptr)
        return true;

    // form the search key
    strcpy(strBuffer, "[");
    strcat(strBuffer, key);
    strcat(strBuffer,"]");
    _strupr(strBuffer);  // keywords must be upper case

    // find the key heading
	pCursor = strstr(pConfig,strBuffer);
	if (pCursor==nullptr)
		return false;

    // find the closing bracket of key heading
	pCursor = strchr(pCursor,']');
	if (pCursor==nullptr)
		return false;
	pCursor++;

    return true;
}

//-----------------------------------
/// extract a number from the config string
// pass the key to find the first number in the list, else nullptr to find the next number in the list
// returns 0 if no number is found, else returns the number
long CConfig::GetLong(char *key)
//-----------------------------------
{
    if (!FindKey(key))
		return 0;


    // look for start of number or end of key
	while (*pCursor && !isdigit(*pCursor) && *pCursor != '[')	
		pCursor++;

	int c = 0;
	while (*pCursor && isdigit(*pCursor) && c < (int)sizeof(strBuffer))
		strBuffer[c++] = *(pCursor++);
	strBuffer[c] = '\0';

	return atol(strBuffer);
}

//-----------------------------------
/// extract string (in double-quotes) from the list.
// pass the key to find the first string in the list, else nullptr to find the next string in the list
// returns nullptr if no string found, else returns a temporary copy of the string (without quotes)
char * CConfig::GetString(char *key)
//-----------------------------------
{
    if (!FindKey(key))
		return nullptr;


    // look for start of string or end of key
	while (*pCursor && *pCursor != '"' && *pCursor != '[')	
		pCursor++;
	if (*(pCursor++) != '"')
		return nullptr;

    // until closing quote
    int c = 0;
	while (*pCursor && c < (int)sizeof(strBuffer))	
	{
		strBuffer[c++] = *pCursor;									// extract string
		if (*pCursor++ == '"')
		{
			strBuffer[--c] = '\0';
			return strBuffer;
		}
	}

	return nullptr;
}


};
#ifdef EXTERNAL_DISTRO
};
#endif

