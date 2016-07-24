// ======================================================================
//
// OsFile.cpp
// Copyright 2002, Sony Online Entertainment Inc.
// All Rights Reserved.
//
// ======================================================================

#include "sharedFile/FirstSharedFile.h"
#include "sharedFile/OsFile.h"

#ifdef _DEBUG
#include "sharedDebug/PerformanceTimer.h"
#endif

namespace OsFileNamespace
{
	float ms_time;
}
using namespace OsFileNamespace;

// ======================================================================

void OsFile::install()
{

}

// ----------------------------------------------------------------------

float OsFile::getSpentTime()
{
	float const result = ms_time;
	ms_time = 0.0f;
	return result;
}

// ----------------------------------------------------------------------

bool OsFile::exists(const char *fileName)
{
	NOT_NULL(fileName);
	DWORD attributes = GetFileAttributes(fileName);

#if _MSC_VER < 1300
	const DWORD INVALID_FILE_ATTRIBUTES = 0xffffffff;
#endif
	return (attributes != INVALID_FILE_ATTRIBUTES && (attributes & FILE_ATTRIBUTE_DIRECTORY) == 0);
}

// ----------------------------------------------------------------------

int OsFile::getFileSize(const char *fileName)
{
	NOT_NULL(fileName);

#ifdef _DEBUG
	PerformanceTimer t;
	t.start();
#endif

	HANDLE handle = CreateFile(fileName, 0, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (handle == INVALID_HANDLE_VALUE)
		return -1;

	int const size = GetFileSize(handle, NULL);
	CloseHandle(handle);

#ifdef _DEBUG
	t.stop();
	ms_time += t.getElapsedTime();
#endif
	return size;
}

// ----------------------------------------------------------------------

OsFile *OsFile::open(const char *fileName, bool randomAccess)
{
#ifdef _DEBUG
	PerformanceTimer t;
	t.start();
#endif

	// attempt to open the file
	HANDLE handle = CreateFile(fileName, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL | (randomAccess ? FILE_FLAG_RANDOM_ACCESS : 0), NULL);

	// check to make sure the file opened sucessfully
	if (handle == INVALID_HANDLE_VALUE)
		return NULL;

#ifdef _DEBUG
	t.stop();
	ms_time += t.getElapsedTime();
#endif

	return new OsFile(handle);
}

// ----------------------------------------------------------------------

OsFile::OsFile(HANDLE handle)
: m_handle(handle),
	m_offset(0)
{
}

// ----------------------------------------------------------------------

OsFile::~OsFile()
{
#ifdef _DEBUG
	PerformanceTimer t;
	t.start();
#endif

	CloseHandle(m_handle);

#ifdef _DEBUG
	t.stop();
	ms_time+= t.getElapsedTime();
#endif
}

// ----------------------------------------------------------------------

int OsFile::length() const
{
	return GetFileSize(m_handle, NULL);
}

// ----------------------------------------------------------------------

void OsFile::seek(int newFilePosition)
{
#ifdef _DEBUG
	PerformanceTimer t;
	t.start();
#endif

	if (m_offset != newFilePosition)
	{
		const DWORD result = SetFilePointer(m_handle, newFilePosition, NULL, FILE_BEGIN);
		DEBUG_FATAL(static_cast<int>(result) != newFilePosition, ("SetFilePointer failed"));
		UNREF(result);
		m_offset = newFilePosition;
	}

#ifdef _DEBUG
	t.stop();
	ms_time += t.getElapsedTime();
#endif
}

// ----------------------------------------------------------------------

int OsFile::read(void *destinationBuffer, int numberOfBytes)
{
#ifdef _DEBUG
	PerformanceTimer t;
	t.start();
#endif

	DWORD amountReadDword;
	BOOL result = ReadFile(m_handle, destinationBuffer, static_cast<uint>(numberOfBytes), &amountReadDword, NULL);

// miles crasher hack
#if 0
	FATAL(!result, ("FileStreamerThread::processRead ReadFile failed to read '%d' bytes with error '%d'", static_cast<uint>(numberOfBytes), GetLastError()));
#else
	if(!result)
	{
		if(GetLastError() == 998) // access violation - buffer coming from miles hosed
		{
			WARNING(true,("FileStreamerThread::processRead ReadFile failed to read '%d' bytes with error '%d'", static_cast<uint>(numberOfBytes), GetLastError()));
			return 0;
		}
		else
		{
			FATAL(true, ("FileStreamerThread::processRead ReadFile failed to read '%d' bytes with error '%d'", static_cast<uint>(numberOfBytes), GetLastError()));
		}
	}
#endif
// end miles crasher hack

#ifdef _DEBUG
	t.stop();
	ms_time += t.getElapsedTime();
#endif

	m_offset += static_cast<int>(amountReadDword);
	return static_cast<int>(amountReadDword);
}

// ======================================================================
