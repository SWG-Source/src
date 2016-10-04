//========================================================================
//
// File.cpp - file reader/writer
//
// copyright 2001 Sony Online Entertainment
//
//========================================================================

#include "sharedTemplateDefinition/FirstSharedTemplateDefinition.h"
#include "File.h"
#include "TemplateGlobals.h"

#ifdef WIN32
#include <share.h>
#endif


//========================================================================
// File static variables

Filename File::m_basePath;
File::FunctionPtr File::m_callBack = nullptr;

//========================================================================
// File functions

/**
 * Opens a file for reading or writing.
 *
 * @param filename		name of file to open (relative to base path)
 * @param mode			read/write mode, see fopen()
 *
 * @return true if the file was opened, false if not
 */
bool File::open(const char *filename, const char *mode)
{
	NOT_NULL(filename);
	NOT_NULL(mode);

	close();

	m_filename.clear();
	m_filename.setName(filename);
	m_filename.prependPath(m_basePath);

#ifdef WIN32
	m_fp = _fsopen(m_filename, mode, _SH_DENYRW);
#else
	// @todo: find an equivalent function for Linux
	m_fp = fopen(m_filename, mode);
#endif
	if (m_fp != nullptr)
	{
		m_currentLine = 0;
		return true;
	}
	else
	{
		const char * errstr = strerror(errno);
		if (errstr != nullptr)
		{
			m_filename.clear();
			printError(errstr);
		}
	}
	return false;
}	// File::open

/**
 * Tests if a file exists.
 *
 * @param filename		the file to test
 *
 * @return true if the file exists, false if not
 */
bool File::exists(const char *filename)
{
	if (filename == nullptr)
		return false;

#if defined(WIN32)

	DWORD attributes = GetFileAttributes(filename);
 #if _MSC_VER < 1300
	const DWORD INVALID_FILE_ATTRIBUTES = 0xffffffff;
 #endif
	return (attributes != INVALID_FILE_ATTRIBUTES && (attributes & FILE_ATTRIBUTE_DIRECTORY) == 0);

#elif defined(linux)

	return FileExists(filename);

#else
	return false;
#endif
}	// File::exists

/**
 * Reads the next line of a file.
 *
 * @param fp			file to read from
 * @param buffer		buffer to read the line in to
 * @param bufferSize	size of buffer
 *
 * @return the size of the string, or -1 on eof or -2 on error
 */
int File::readRawLine(char *buffer, int bufferSize)
{
	NOT_NULL(m_fp);
	NOT_NULL(buffer);

	++m_currentLine;
	if (fgets(buffer, bufferSize, m_fp) == nullptr)
	{
		if (feof(m_fp))
			return -1;
		else
		{
			printError("error reading file.");
			return -2;
		}
	}

	// if buffer doen't end in '\n', the line is too long
	if (static_cast<int>(strlen(buffer)) == bufferSize - 1 && 
		buffer[bufferSize - 2] != '\n')
	{
		printError("line too long");
		return -2;
	}

	return strlen(buffer);
}	// File::readRawLine

/**
 * Reads the next line of a file, strips leading and trailing whitespace.
 *
 * @param fp			file to read from
 * @param buffer		buffer to read the line in to
 * @param bufferSize	size of buffer
 *
 * @return the size of the string, or -1 on eof or -2 on error
 */
int File::readLine(char *buffer, int bufferSize)
{
	NOT_NULL(m_fp);
	NOT_NULL(buffer);

	for (;;)
	{
		++m_currentLine;
		if (fgets(buffer, bufferSize, m_fp) == nullptr)
		{
			if (feof(m_fp))
				return -1;
			else
			{
				printError("error reading file.");
				return -2;
			}
		}

		// if buffer doen't end in '\n', the line is too long
		if (static_cast<int>(strlen(buffer)) == bufferSize - 1 && 
			buffer[bufferSize - 2] != '\n')
		{
			printError("line too long");
			return -2;
		}

		int length = strip(buffer);
		if (length != 0)
			return length;
	}
}	// File::readLine

/**
 * fprintf functionality.
 */
int File::print(const char *format, ...)
{
	NOT_NULL(m_fp);

	va_list argptr;
	va_start(argptr, format);

	int ret = vfprintf(m_fp, format, argptr);

	va_end(argptr);
	
	return ret;
}	// File::print
