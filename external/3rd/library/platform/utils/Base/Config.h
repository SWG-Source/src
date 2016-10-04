/*
; Example syntax of a config file.
# This is a comment to end of line.
; This is a comment too....both comments are to end of line
[KEYWORD]   ; all keywords must be upper case to be recognized

; single number, with code example
# long number = config.GetLong("PORT");
[PORT]  5999    ; common single-value case

; list of strings, with code example
# config.FindKey("NUMBERS");
# while (number = config.GetLong(nullptr))
#    YourHandleNumber(number);
[NUMBERS]   100
    200, 300    ; formatting is very flexible as long as numbers are delimited by non-numbers
    400 500
    600         ; This is badly formatted for example purposes only

; single string, with code example
# strcpy(mystring, config.GetString("STRING"));
[STRING]  "This is a string"

; list of strings, with code example
# config.FindKey("HOSTS");
# while (string = config.GetString(nullptr))
#    YourHandleString(string);
[HOSTS]	
    "206.19.151.173:5999"
    "206.19.151.173:5998"
    "206.19.151.173:2000"

*/

#ifndef _CONFIG_H
#define _CONFIG_H

#include <stdio.h>

#ifdef EXTERNAL_DISTRO
namespace NAMESPACE 
{

#endif
namespace Base 
{

    class CConfig
    {
    public:
        // constructor, no file loaded
        CConfig();

        // version of constructor that calls LoadFile(file)
        CConfig(char * file);   

        // destructor, will delete all buffers
        ~CConfig();

        // copy text config file into memory. Required to use extraction methods.
        // if called with a different filename, existing file will be discarded.
        bool LoadFile(char * file);     

        // delete memory buffer of file
        void UnloadFile();              

        // set cursor in file based on key name. Alternate way to select first key when extracting lists.
        // returns TRUE if key is found.
        bool FindKey(char *key);            

        // extract string from config.  If key is nullptr, extract next string in list, else extract first string.
        // if key was not found, returns nullptr
        char * GetString(char *key = nullptr); 

        // extract number from config.  If key is nullptr, extract next number in list, else extract first number.
        // if key was not found, returns 0
        long GetLong(char *key = nullptr);     // extract number from config.

		// indicate if config file has been loaded
		inline bool FileLoaded()		{ return pConfig == nullptr ? false : true; }

    private:
        char * pConfig;     // pointer to file memory
        char * pCursor;    // current cursor into file memory
        char strBuffer[8196];  // buffer for GetString return pointer -- reused
    };

    inline CConfig::CConfig()
    {
        pConfig = nullptr;
        pCursor = nullptr;
    }

    inline CConfig::CConfig(char * file)
    {
        pConfig = nullptr;
        pCursor = nullptr;
        LoadFile(file);
    }

    inline CConfig::~CConfig()
    {
        UnloadFile();
    }

};
#ifdef EXTERNAL_DISTRO
};
#endif

#endif
