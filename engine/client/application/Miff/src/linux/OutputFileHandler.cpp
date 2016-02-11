//===========================================================================
//
//    FILENAME: OutputFileHandler.cpp
//   COPYRIGHT: (C) 1999 BY Bootprint Entertainment
//
// DESCRIPTION: file handler for Output file (IFF file)
//      AUTHOR: Hideki Ikeda
//        DATE: 1/13/99 4:52:42 PM
//
//===========================================================================

#include "sharedFoundation/FirstSharedFoundation.h"
#include "OutputFileHandler.h"

#include "sharedFile/Iff.h"

//===========================================================================
// Constructor

OutputFileHandler::OutputFileHandler(const char *filename)
{
	outputIFF = new Iff(MAXIFFDATASIZE);
	outFilename = nullptr;

	setCurrentFilename(filename);
}

void OutputFileHandler::setCurrentFilename(const char *filename)
{
	if (outFilename)
		delete [] outFilename;

	outFilename = new char[strlen(filename)+1];
	strcpy(outFilename, filename);
}	

//---------------------------------------------------------------------------
// Destructor

OutputFileHandler::~OutputFileHandler(void)
{
	if (outputIFF && outFilename)
	{
		delete outputIFF;
		delete [] outFilename;
	}

	outputIFF = nullptr;
}


//---------------------------------------------------------------------------
// begins a new FORM in the IFF
//
// Return Value:
//   bool - true == success
//
// See Also:
//   Iff::insertForm()

void OutputFileHandler::insertForm(
		const char *tag
		)
{
	Tag formTag = convertStrToTag(tag);
	outputIFF->insertForm(formTag);
}

//---------------------------------------------------------------------------
// begins a new CHUNK in the IFF
//
// See Also:
//   Iff::insertChunk()

void OutputFileHandler::insertChunk(
		const char *tag
		)
{
	Tag chunkTag = convertStrToTag(tag);
	outputIFF->insertChunk(chunkTag);
}


//---------------------------------------------------------------------------
// converts string (4 bytes) form into Tag format
//
// Return Value:
//   Tag
//
// Remarks:
//   currently, this code is machine dependant code (non portable) and it assumes little endian
//
// See Also:
//   Tag

Tag OutputFileHandler::convertStrToTag(
		const char *str
		)
{
	// prepare for hack-o-rama.  It is byte order dependant, thus not portable ^_^
	Tag retVal = str[3] + (str[2] * 0x100) + (str[1] * 0x10000) + (str[0] * 0x1000000);

	return(retVal);
}


//---------------------------------------------------------------------------
// adds new chunk data into the current chunk it is in
//
// See Also:
//   Iff::insertChunkData()
//

void OutputFileHandler::insertChunkData(
		void *data,
		int length
		)
{
	outputIFF->insertChunkData(data, length);
}

//---------------------------------------------------------------------------
// exits current FORM section we are in
//
// See Also:
//   Iff::exitForm()

void OutputFileHandler::exitForm(void)
{
	outputIFF->exitForm();
}

//---------------------------------------------------------------------------
// exits current CHUNK we are in
//
// See Also:
//   Iff::exitChunk()

void OutputFileHandler::exitChunk(void)
{
	outputIFF->exitChunk();
}


//---------------------------------------------------------------------------
// Calls Iff:write()
//
// Return Value:
//
//   True if the Iff was successfully written, otherwise false
//
// See Also:
//   Iff::write()

bool OutputFileHandler::writeBuffer(void)
{
	if (outputIFF && outFilename)
		return outputIFF->write(outFilename, true);

	return false;
}

//===========================================================================

