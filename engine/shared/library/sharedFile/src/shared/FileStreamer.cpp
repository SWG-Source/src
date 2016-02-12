// ======================================================================
//
// FileStreamer.cpp
// Portions copyright 1998 Bootprint Entertainment
// Portions copyright 2002 Sony Online Entertainment
// All Rights Reserved
//
// ======================================================================

#include "sharedFile/FirstSharedFile.h"
#include "sharedFile/FileStreamer.h"

#include "sharedFile/FileStreamerThread.h"
#include "sharedFile/OsFile.h"
#include "sharedFoundation/CrashReportInformation.h"
#include "sharedFoundation/ExitChain.h"
#include "sharedFoundation/Production.h"
#include "sharedFoundation/PerThreadData.h"
#include "sharedFoundation/MemoryBlockManager.h"
#include "sharedSynchronization/Gate.h"

#include <string>

// ======================================================================

bool FileStreamer::ms_installed;
bool FileStreamer::ms_useThread;	

// ======================================================================

namespace FileStreamerNamespace
{
	bool reportModDirectory(char const * const fileName ,char const * const directory, char const * const description);
}

using namespace FileStreamerNamespace;

// ======================================================================
// Install the FileStreamer

void FileStreamer::install(bool useThread)
{
	DEBUG_FATAL(ms_installed, ("FileStreamer::install already installed"));
	ms_installed = true;
	ms_useThread = useThread;
	File::install(ms_useThread);

	if (ms_useThread)
		FileStreamerThread::install();

	ExitChain::add(FileStreamer::remove, "FileStreamer::remove", 0, true);
}

// ----------------------------------------------------------------------
/**
 * Remove the FileStreamer subsystem.
 * 
 * This function will fatal if it has to wait more than MaxTimeToWait
 * for the file streamer thread to shut down.
 */

void FileStreamer::remove(void)
{
	DEBUG_FATAL(!ms_installed, ("FileStreamer::remove not installed"));
	ms_installed = false;
}

// ----------------------------------------------------------------------
/**
 * Check if a file exists.
 * 
 * This routine does not indicate that the file is actually readable by
 * the process.
 * 
 * @param fileName  File name to check existence of
 * @return True if the file exists, otherwise false.
 */

bool FileStreamer::exists(const char *fileName)
{
	return OsFile::exists(fileName);
}

// ----------------------------------------------------------------------
/**
 * Check if a file exists.
 * 
 * This routine does not indicate that the file is actually readable by
 * the process.
 * 
 * @param fileName  File name to check existence of
 * @return True if the file exists, otherwise false.
 */

int FileStreamer::getFileSize(const char *fileName)
{
	return OsFile::getFileSize(fileName);
}

// ----------------------------------------------------------------------
/**
 * Open a file.
 * 
 * @param fileName  [In]  File name to open
 * @param handle  [Out] Handle to the opened file
 * @return True if the file was successfully opened, otherwise false.
 */

FileStreamer::File *FileStreamer::open(const char *fileName, bool randomAccess)
{
	DEBUG_FATAL(!ms_installed, ("not installed"));
	DEBUG_FATAL(!fileName, ("file name nullptr"));

	OsFile *osFile = OsFile::open(fileName, randomAccess);
	if (!osFile)
		return nullptr;

#if PRODUCTION
	// Stop checking the fileName if one of these returns true
	reportModDirectory(fileName, "ui/", "UI") ||
	reportModDirectory(fileName, "texture/", "Texture") ||
	reportModDirectory(fileName, "effect/", "Effect") ||
	reportModDirectory(fileName, "appearance/", "Appearance") ||
	reportModDirectory(fileName, "music/", "Music") ||
	reportModDirectory(fileName, "sound/", "Sound") ||
	reportModDirectory(fileName, "camera/", "Camera") ||
	reportModDirectory(fileName, "shader/", "Shader");
#endif

	// return the opened file
	return new FileStreamer::File(osFile);
}

// ======================================================================

MemoryBlockManager  *FileStreamer::File::ms_memoryBlockManager;
bool                              FileStreamer::File::ms_useThread;

// ======================================================================

void FileStreamer::File::install(bool useThread)
{
	ms_memoryBlockManager = new MemoryBlockManager("FileStreamer::File::memoryBlockManager", true, sizeof(FileStreamer::File), 0, 0, 0);
	ExitChain::add(&remove, "FileStreamer::File");
	ms_useThread = useThread;
}

// ----------------------------------------------------------------------

void FileStreamer::File::remove()
{
	delete ms_memoryBlockManager;
	ms_memoryBlockManager = nullptr;
}

// ----------------------------------------------------------------------

void *FileStreamer::File::operator new(size_t size)
{
	UNREF(size);
	NOT_NULL (ms_memoryBlockManager);
	DEBUG_FATAL(size != sizeof(FileStreamer::File), ("incorrect File size"));
	DEBUG_FATAL(size != static_cast<size_t> (ms_memoryBlockManager->getElementSize()), ("installed with bad size"));
	return ms_memoryBlockManager->allocate();
}

// ----------------------------------------------------------------------

void FileStreamer::File::operator delete(void *pointer)
{
	NOT_NULL (ms_memoryBlockManager);
	ms_memoryBlockManager->free(pointer);
}

// ----------------------------------------------------------------------

FileStreamer::File::File(OsFile *osFile)
:
	m_osFile(osFile),
	m_offset(0)
{
}

// ----------------------------------------------------------------------

FileStreamer::File::~File()
{
	close();
}

// ----------------------------------------------------------------------

bool FileStreamer::File::isOpen() const
{
	return m_osFile != nullptr;
}

// ----------------------------------------------------------------------

int FileStreamer::File::length() const
{
	DEBUG_FATAL(!isOpen(), ("file is not open"));
	return m_osFile->length();
}

// ----------------------------------------------------------------------

int FileStreamer::File::read(int offset, void *destinationBuffer, int numberOfBytes, AbstractFile::PriorityType priority)
{
	DEBUG_FATAL(!isOpen(), ("file is not open"));

	if (!ms_useThread)
	{
		m_osFile->seek(offset);
		return m_osFile->read(destinationBuffer, numberOfBytes);
	}

	int returnValue = -1;

	Gate *gate = PerThreadData::getFileStreamerReadGate();

	// build the request
	FileStreamerThread::Request *newRequest = new FileStreamerThread::Request;
	newRequest->type           = FileStreamerThread::Request::Read;
	newRequest->osFile         = m_osFile;
	newRequest->buffer         = destinationBuffer;
	newRequest->offset         = offset;
	newRequest->bytesToBeRead  = numberOfBytes;
	newRequest->bytesRead      = 0;
	newRequest->gate           = gate;
	newRequest->priority       = priority;
	newRequest->returnValue    = &returnValue;

	// submit the request
	FileStreamerThread::submitRequest(newRequest);

	// wait for the request to complete
	gate->wait();
	gate->close();

	DEBUG_FATAL(returnValue < 0, ("failed to set the return value"));
	return returnValue;
}

// ----------------------------------------------------------------------

void FileStreamer::File::close()
{
	if (isOpen())
	{
		delete m_osFile;
		m_osFile = nullptr;
	}
}

// ======================================================================

bool FileStreamerNamespace::reportModDirectory(char const * const fileName ,char const * const directory, char const * const description) 
{
	if (strstr(fileName, directory))
	{
		DEBUG_REPORT_LOG(true, ("Loaded %s off filesystem: %s\n", description, fileName)); 
		CrashReportInformation::addStaticText("%s file: %s\n", description, fileName); 
		return true;
	}
	return false;
}

// ======================================================================
