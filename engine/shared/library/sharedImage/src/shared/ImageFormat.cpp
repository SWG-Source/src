// ======================================================================
//
// ImageFormat.cpp
// copyright 2001 Sony Online Entertainment
// 
// ======================================================================

#include "sharedImage/FirstSharedImage.h"
#include "sharedImage/ImageFormat.h"

#include "sharedFile/TreeFile.h"

// ======================================================================

bool ImageFormat::loadFileCreateBuffer(const char *filename, uint8 **buffer, int *bufferSize)
{
	//-- validate args
	NOT_NULL(filename);
	NOT_NULL(buffer);
	NOT_NULL(bufferSize);

	if (!*filename)
	{
		REPORT_LOG(true, ("empty filename\n"));
		return false;
	}

	//-- open file
	AbstractFile *fileInterface = TreeFile::open(filename, AbstractFile::PriorityData, true);
	if(!fileInterface)
	{
		REPORT_LOG(true, ("failed to open file [%s]\n", filename));
		return false;
	}

	//-- create buffer
	*bufferSize = fileInterface->length();
	*buffer     = NON_NULL(new uint8[static_cast<uint>(*bufferSize)]);

	//-- load the buffer
	const int readBytes = fileInterface->read(*buffer, *bufferSize);
	delete fileInterface;
	if (readBytes != *bufferSize)
	{
		REPORT_LOG(true, ("error: reported file size = %d, we read %d\n", *bufferSize, readBytes));
		return false;
	}

	//-- no errors
	return true;
}

// ======================================================================

ImageFormat::ImageFormat()
{
}

// ----------------------------------------------------------------------

ImageFormat::~ImageFormat()
{
}

// ======================================================================
