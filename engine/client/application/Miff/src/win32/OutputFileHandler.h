#ifndef __OUTPUTFILEHANDLER_H__
#define __OUTPUTFILEHANDLER_H__

//===========================================================================
//
//    FILENAME: OutputFileHandler.h [C:\Projects\new\tools\src\miff\src\]
//   COPYRIGHT: (C) 1999 BY Bootprint Entertainment
//
// DESCRIPTION: file handler for output files (IFF file format)
//      AUTHOR: Hideki Ikeda
//        DATE: 1/13/99 4:55:56 PM
//
//     HISTORY: 1/13/99 [HAI] - File created
//            : 
//
//===========================================================================

//============================================================== #includes ==

//========================================================= class typedefs ==
#include "sharedFile/Iff.h"

//====================================================== class definitions ==
class OutputFileHandler
{
//------------------------------
//--- public var & functions ---
//------------------------------
public:     // functions
	OutputFileHandler(const char *filename);
	~OutputFileHandler(void);
	bool writeBuffer(void);

	void insertForm(const char *tagName);
	void insertChunk(const char *tagName);
	void insertChunkData(void *data, int length);
	void exitForm(void);
	void exitChunk(void);

	void setCurrentFilename(const char *fname);

public:     // vars


	//-------------------------------
	//--- member vars declaration ---
	//-------------------------------
protected:  // vars
	Iff		* outputIFF;
	char	*outFilename;

enum{
		MAXIFFDATASIZE = 8192			// allocate 8K of memory for a starter
	};

private:    // vars

	//-----------------------------------
	//--- member function declaration ---
	//-----------------------------------
protected:  // functions

private:    // functions
	Tag convertStrToTag(const char *str);

};

//===========================================================================
//========================================================= inline methods ==
//===========================================================================


//===========================================================================
//============================================================ End-of-file ==
//===========================================================================
#else 
    #ifdef DEBUG 
    #pragma message("OutputFileHandler.h included more then once!") 
    #endif 
#endif  // ifndef __H__
