// ======================================================================
//
// OsFile.h
// Copyright 2002, Sony Online Entertainment Inc.
// All Rights Reserved.
//
// ======================================================================

#ifndef INCLUDED_OsFile_H
#define INCLUDED_OsFile_H

// ======================================================================

class OsFile
{
public:

	static void install();

	static bool    exists(const char *fileName);
	static int     getFileSize(const char *fileName);
	static OsFile *open(const char *fileName, bool randomAccess=false);

public:

	~OsFile();

	int  length() const;
	int  tell() const;
	void seek(int newFilePosition);
	int  read(void *destinationBuffer, int numberOfBytes);

private:

	OsFile(int handle, char *fileName);

	OsFile();
	OsFile(const OsFile &);
	OsFile &operator =(const OsFile &);

private:

	int    m_handle;
	int    m_length;
	int    m_offset;
	char  *m_fileName;
};

// ======================================================================

#endif
