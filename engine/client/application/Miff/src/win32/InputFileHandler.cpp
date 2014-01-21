//===========================================================================
//
//    FILENAME: InputFileHandler.cpp [C:\Projects\new\tools\src\miff\src\]
//   COPYRIGHT: (C) 1999 BY Bootprint Entertainment
//
// DESCRIPTION: file handler for input files (standard flat text files)
//      AUTHOR: Hideki Ikeda
//        DATE: 1/13/99 4:53:31 PM
//
//     HISTORY: 1/13/99 [HAI] - File created
//            : 
//
//    FUNCTION: InputFileHandler()			constructor
//            : ~InputFileHandler()			destructor
//            : 
//
//===========================================================================

//========================================================== include files ==
#include "sharedFoundation/FirstSharedFoundation.h"
#include "InputFileHandler.h"

#include "sharedFile/TreeFile.h"
//#include "sharedFile/Iff.h"

//================================================= static vars assignment ==



//---------------------------------------------------------------------------
// Constructor
//
// Remarks:
//   
//
// See Also:
//   
//
// Revisions and History:
//   1/13/99 [HAI] - created
//
InputFileHandler::InputFileHandler(const char *infilename)
{
	TreeFile::addSearchAbsolute(0);         // search current working directory
	
	file = TreeFile::open(infilename, AbstractFile::PriorityData, true);

}


//---------------------------------------------------------------------------
// Destructor
//
// Remarks:
//   
//
// See Also:
//   
//
// Revisions and History:
//   1/13/99 [HAI] - created
//
InputFileHandler::~InputFileHandler(void)
{
	if(file)
		delete file;
}


//---------------------------------------------------------------------------
// reads a file stream into specified buffer of the size passed
//
// Return Value:
//   actual size read (signed int)
//
// Remarks:
//   
//
// See Also:
//   Treefile::read()
//
// Revisions and History:
//   1/13/99 [HAI] - created
//
const int InputFileHandler::read(
		void *sourceBuffer,				// pointer to the buffer
		int   bufferSize				// number of BYTES to be read
		)
{
	int		retVal = -1;				// assume fileHandle is NOT valid

	if (file)
		retVal = file->read(sourceBuffer, bufferSize);

	return(retVal);
}

//---------------------------------------------------------------------------
// Deletes a file
//
// Return Value:
//   whatever DeleteFile() returns
//   if fileHandle != -1, it assumes that the fileHandle passed belonged to
//   this filename, and therefore, it will attempt to close the file and
//   set it to 0.
//
// Remarks:
//   calls DeleteFile() found in windows.h
//   InputFileHandler does NOT have any way to validate that the handle
//   passed belongs to the filename that it wants to be deleted.  So use
//   it with caution
//
// See Also:
//   windows.h
//
// Revisions and History:
//   1/13/99 [HAI] - created
//
int InputFileHandler::deleteFile(
		const char 	*filename,
		bool		deleteHandleFlag
		)
{
	if (deleteHandleFlag && file)
	{
		delete file;
		file = NULL;
	}
	return(DeleteFile(filename));
}


//===========================================================================
//============================================================ End-of-file ==
//===========================================================================

