// ======================================================================
//
// OsFile.cpp
// Copyright 2002, Sony Online Entertainment Inc.
// All Rights Reserved.
//
// ======================================================================

#include "sharedFile/FirstSharedFile.h"
#include "sharedFile/OsFile.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>

// ======================================================================

void OsFile::install()
{
}

// ----------------------------------------------------------------------

bool OsFile::exists(const char *fileName)
{
	struct stat statBuffer;
	if (stat(fileName, &statBuffer) != 0)
		return false;

	if (S_ISDIR(statBuffer.st_mode))
		return false;

	return true;
}

// ----------------------------------------------------------------------

int OsFile::getFileSize(const char *fileName)
{
	struct stat statBuffer;
	int fileSize = 0;

	if (stat(fileName, &statBuffer) == 0)
	{
		fileSize = static_cast<int>(statBuffer.st_size);
	}
	return fileSize;
}

// ----------------------------------------------------------------------

OsFile *OsFile::open(const char *fileName, bool randomAccess)
{
	UNREF(randomAccess);

	if (!exists(fileName))
		return 0;

	// attempt to open the file
	const int handle = ::open(fileName, O_RDONLY);
	FATAL(handle < 0, ("OsFile::open failed to open file %s, errno=%d, which does exist.", fileName, errno));

	return new OsFile(handle, DuplicateString(fileName));
}

// ----------------------------------------------------------------------

OsFile::OsFile(int handle, char *fileName)
:
	m_handle(handle),
	m_length(0),
	m_offset(0),
	m_fileName(fileName)
{
	m_length = lseek(m_handle, 0, SEEK_END);
	lseek(m_handle, 0, SEEK_SET);
}

// ----------------------------------------------------------------------

OsFile::~OsFile()
{
	close(m_handle);
	delete [] m_fileName;
}

// ----------------------------------------------------------------------

int OsFile::length() const
{
	return m_length;
}

// ----------------------------------------------------------------------

void OsFile::seek(int newFilePosition)
{
	if (m_offset != newFilePosition)
	{
		const int result = lseek(m_handle, newFilePosition, SEEK_SET);
		DEBUG_FATAL(result != newFilePosition, ("SetFilePointer failed"));
		UNREF(result);
		m_offset = newFilePosition;
	}
}

// ----------------------------------------------------------------------

int OsFile::read(void *destinationBuffer, int numberOfBytes)
{
	int result = 0;
	do
	{
		result = ::read(m_handle, destinationBuffer, numberOfBytes);
		DEBUG_FATAL((result < 0 && errno != EAGAIN), ("Read failed for %s: %d %d %s", m_fileName, result, errno, strerror(errno)));
	} while (result < 0);

	m_offset += result;
	return result;
}

// ======================================================================
