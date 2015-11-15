#ifndef __INPUTFILEHANDLER_H__
#define __INPUTFILEHANDLER_H__

//===========================================================================
//
//    FILENAME: InputFileHandler.h [C:\Projects\new\tools\src\miff\src\]
//   COPYRIGHT: (C) 1999 BY Bootprint Entertainment
//
// DESCRIPTION: file handler for input files (flat text files)
//      AUTHOR: Hideki Ikeda
//        DATE: 1/13/99 4:55:15 PM
//
//     HISTORY: 1/13/99 [HAI] - File created
//            : 
//
//===========================================================================

//============================================================== #includes ==

//========================================================= class typedefs ==

//====================================================== class definitions ==

class AbstractFile;

class InputFileHandler
{
//------------------------------
//--- public var & functions ---
//------------------------------
public:     // functions
	InputFileHandler(const char *infilename);
	~InputFileHandler(void);

	const int read(void *sourceBuffer, int bufferSize);
	int       deleteFile(const char * filename, bool deleteHandleFlag = false);

public:     // vars


	//-------------------------------
	//--- member vars declaration ---
	//-------------------------------
protected:  // vars
	AbstractFile *file;

private:    // vars

	//-----------------------------------
	//--- member function declaration ---
	//-----------------------------------
protected:  // functions

private:    // functions
	void close(void);			// close the input file called by destructor

};

//===========================================================================
//========================================================= inline methods ==
//===========================================================================


//===========================================================================
//============================================================ End-of-file ==
//===========================================================================
#else 
    #ifdef DEBUG 
    #pragma message("InputFileHandler.h included more then once!") 
    #endif 
#endif  // ifndef __H__
