// ======================================================================
//
// FileStreamerFile.h
// Copyright 2002 Sony Online Entertainment
// All rights reserved.
//
// ======================================================================

#ifndef INCLUDED_FileStreamerFile_H
#define INCLUDED_FileStreamerFile_H

// ======================================================================

class MemoryBlockManager;
class OsFile;

#include "fileInterface/AbstractFile.h"
#include "sharedFile/FileStreamer.h"

// ======================================================================

class FileStreamerFile : public AbstractFile
{
public:

	static void install();
	static void *operator new(size_t size);
	static void  operator delete(void *memory);

public:

	FileStreamerFile(PriorityType priority, FileStreamer::File &file);
	FileStreamerFile(PriorityType priority, FileStreamer::File &file, int baseOffset, int length);
	virtual ~FileStreamerFile();

	virtual bool isOpen() const;
	virtual int  length() const;
	virtual int  tell() const;
	virtual bool seek(SeekType seekType, int offset);
	virtual int  read(void *destinationBuffer, int numberOfBytes);

	virtual int  write(int numberOfBytes, const void *sourceBuffer);
	virtual void flush();

	virtual void close();
	
private:

	static MemoryBlockManager *ms_memoryBlockManager;

private:

	static void remove();

private:

	FileStreamerFile();
	FileStreamerFile(const FileStreamerFile &);
	FileStreamerFile &operator =(const FileStreamerFile &);

private:

	FileStreamer::File *m_file;
	const bool          m_owner;
	const int           m_baseOffset;
	const int           m_length;
	int                 m_offset;
};

// ======================================================================

#endif

