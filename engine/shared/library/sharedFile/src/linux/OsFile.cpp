// ======================================================================
//
// OsFile.cpp
// Copyright 2002, Sony Online Entertainment Inc.
// All Rights Reserved.
//
// ======================================================================

#include "sharedFile/FirstSharedFile.h"
#include "sharedFile/OsFile.h"

#include <cerrno>
#include <cstring>
#include <limits>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>

// ======================================================================

namespace OsFileNamespace
{
	// The file API is 32-bit by contract: AbstractFile::length()/tell()
	// return int, seek() takes int, and TRE/TOC offsets are 32-bit on disk.
	// A file whose size cannot be represented in that contract is refused
	// rather than having its length wrap.
	off_t const cs_maximumFileSize = static_cast<off_t>(std::numeric_limits<int>::max());
}

using namespace OsFileNamespace;

// ======================================================================

void OsFile::install()
{
}

// ----------------------------------------------------------------------

bool OsFile::exists(const char *fileName)
{
	struct stat statBuffer;
	if (stat(fileName, &statBuffer) != 0)
	{
		// A 32-bit off_t cannot describe the file, but the file is there.
		// open() reports and refuses it instead of it silently vanishing.
		return errno == EOVERFLOW;
	}

	if (S_ISDIR(statBuffer.st_mode))
		return false;

	return true;
}

// ----------------------------------------------------------------------

int OsFile::getFileSize(const char *fileName)
{
	// -1 means the file cannot be used, matching the win32 implementation
	// and TreeFile::getFileSize, which treats any size >= 0 as found.
	struct stat statBuffer;
	if (stat(fileName, &statBuffer) != 0 || S_ISDIR(statBuffer.st_mode))
		return -1;

	if (statBuffer.st_size < 0 || statBuffer.st_size > cs_maximumFileSize)
		return -1;

	return static_cast<int>(statBuffer.st_size);
}

// ----------------------------------------------------------------------

OsFile *OsFile::open(const char *fileName, bool randomAccess)
{
	UNREF(randomAccess);

	if (!exists(fileName))
		return 0;

	// attempt to open the file
	const int handle = ::open(fileName, O_RDONLY);
	if (handle < 0 && errno == EOVERFLOW)
	{
		// only possible with a 32-bit off_t; the file exceeds the 32-bit file API
		WARNING(true, ("OsFile::open refusing %s: file is larger than the %d byte limit of the file API", fileName, std::numeric_limits<int>::max()));
		return 0;
	}
	FATAL(handle < 0, ("OsFile::open failed to open file %s, errno=%d, which does exist.", fileName, errno));

	struct stat statBuffer;
	if (fstat(handle, &statBuffer) != 0)
	{
		const int error = errno;
		IGNORE_RETURN(close(handle));
		if (error == EOVERFLOW)
			WARNING(true, ("OsFile::open refusing %s: file is larger than the %d byte limit of the file API", fileName, std::numeric_limits<int>::max()));
		else
			WARNING(true, ("OsFile::open failed to stat %s, errno=%d (%s)", fileName, error, strerror(error)));
		return 0;
	}

	const off_t fileSize = statBuffer.st_size;
	if (fileSize < 0 || fileSize > cs_maximumFileSize)
	{
		IGNORE_RETURN(close(handle));
		WARNING(true, ("OsFile::open refusing %s: file size %lld is larger than the %d byte limit of the file API", fileName, static_cast<long long>(fileSize), std::numeric_limits<int>::max()));
		return 0;
	}

	return new OsFile(handle, DuplicateString(fileName), static_cast<int>(fileSize));
}

// ----------------------------------------------------------------------

OsFile::OsFile(int handle, char *fileName, int length)
:
	m_handle(handle),
	m_length(length),
	m_offset(0),
	m_fileName(fileName)
{
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
