//========================================================================
//
// File.h - file reader/writer
//
// copyright 2001 Sony Online Entertainment
//
//========================================================================

#ifndef _INCLUDED_File_H
#define _INCLUDED_File_H

#include "Filename.h"

#include <cstdio>

class File
{
public:
	explicit File(void);
	explicit File(const char *filename, const char *mode);
	~File();

	static void setBasePath(const char *path);

	const Filename & getFilename(void) const;
	bool isOpened(void) const;

	bool open(const char *filename, const char *mode);
	void close(void);
	bool exists(const char *filename);

	int readRawLine(char *buffer, int bufferSize);
	int readLine(char *buffer, int bufferSize);

	int puts(const char *string);
	int print(const char *format, ...);

	void printWarning(const char *buffer) const;
	void printError(const char *buffer) const;
	void printEolError(void) const;

	typedef void (* FunctionPtr)(const char *error);

	static void setErrorCallBack(FunctionPtr callBack);

private:
	FILE *m_fp;					// file being read from
	Filename m_filename;		// name of file being read
	int m_currentLine;			// current line being read

	static FunctionPtr m_callBack;
	static Filename m_basePath;		// path all files will be opened relative to
};


inline File::File(void) :
	m_fp(nullptr),
	m_currentLine(0)
{
}	// File::File(void)

inline File::File(const char *filename, const char *mode) :
	m_fp(nullptr),
	m_currentLine(0)
{
	open(filename, mode);
}	// File::File(const char *)

inline File::~File()
{
	close();
}	// File::~File

inline void File::setBasePath(const char *path)
{
	m_basePath.setPath(path);
}	// File::setBasePath

inline const Filename & File::getFilename(void) const
{
	return m_filename;
}	// File::getFilename

inline bool File::isOpened(void) const
{
	return (m_fp != nullptr);
}	// File::isOpened

inline void File::close(void)
{
	if (m_fp != nullptr)
	{
		fclose(m_fp);
		m_fp = nullptr;
	}
}	// File::close

inline int File::puts(const char *string)
{
	NOT_NULL(m_fp);
	return fputs(string, m_fp);
}	// File::puts

inline void File::printWarning(const char *buffer) const
{
	NOT_NULL(buffer);

	char error[4096];
	sprintf(error, "WARNING file <%s> line %d: %s\n", m_filename.getName().c_str(), m_currentLine, buffer);

	fprintf(stderr, "%s", error);

	if (m_callBack != nullptr)
	{
		m_callBack(error);
	}
}	// File::printWarning

inline void File::printError(const char *buffer) const
{
	NOT_NULL(buffer);

	char error[4096];
	sprintf(error, "ERROR file <%s> line %d: %s\n", m_filename.getName().c_str(), m_currentLine, buffer);

	fprintf(stderr, "%s", error);

	if (m_callBack != nullptr)
	{
		m_callBack(error);
	}
}	// File::printError

inline void File::printEolError(void) const
{
	printError("unexpected end of line");
}	// File::printEolError

inline void File::setErrorCallBack(FunctionPtr callBack)
{
	m_callBack = callBack;
} // File::setErrorCallBack

#endif	// _INCLUDED_File_H
