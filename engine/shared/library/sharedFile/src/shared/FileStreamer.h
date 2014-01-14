// ======================================================================
//
// FileStreamer.h
// Portions copyright 1998 Bootprint Entertainment
// Portions copyright 2002 Sony Online Entertainment
// All Rights Reserveed
//
// ======================================================================

#ifndef INCLUDED_FileStreamer_H
#define INCLUDED_FileStreamer_H

// ======================================================================

class FileStreamerThread;
class MemoryBlockManager;
class OsFile;

#include "fileInterface/AbstractFile.h"

// ======================================================================
/**
 * Low-level disk access routines.
 *
 * This class spawns a thread to take care of all disk access.
 *
 * This class basically wraps the FileStreamerThread class, which no other
 * class should access.
 *
 * Game programmers likely want to use the TreeFile class to access disk
 * files instead of this class.
 */

class FileStreamer 
{
	friend class FileStreamerThread;

public:

	class File;

public:

	static void install(bool useThread);
	static void remove();

	static bool    exists(const char *fileName);
	static int     getFileSize(const char *fileName);
	static File   *open(const char *fileName, bool randomAccess=false);

private:

	static bool  ms_installed;
	static bool  ms_useThread;
};

// ======================================================================

class FileStreamer::File
{
	friend class FileStreamerThread;

public:

	static void  install(bool useThread);
	static void *operator new(size_t size);
	static void  operator delete(void *memory);

public:

	File(OsFile *osFile);
	~File();

	bool isOpen() const;
	int  length() const;
	int  read(int offset, void *destinationBuffer, int numberOfBytes, AbstractFile::PriorityType priority);
	void close();

private:

	static void remove();

private:

	File();
	File(const File &);
	File &operator =(const File &);

private:

	static MemoryBlockManager *ms_memoryBlockManager;

private:

	static bool ms_useThread;

private:

	OsFile *m_osFile;
	int     m_offset;
};

// ======================================================================

#endif
