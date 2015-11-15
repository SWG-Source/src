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

	static float   getSpentTime();

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

	OsFile(HANDLE handle);
	OsFile(const char *fileName);

private:

	HANDLE m_handle;
	int    m_offset;
};

// ======================================================================

#endif
