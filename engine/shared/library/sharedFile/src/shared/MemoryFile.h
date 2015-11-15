// ======================================================================
//
// MemoryFile.h
// Copyright 2002, Sony Online Entertainment Inc.
// All Rights Reserved.
//
// ======================================================================

#ifndef INCLUDED_MemoryFile_H
#define INCLUDED_MemoryFile_H

// ======================================================================

class MemoryBlockManager;

#include "fileInterface/AbstractFile.h"

// ======================================================================

class MemoryFile : public AbstractFile
{
public:

	static void           install();
	static void *operator new(size_t size);
	static void  operator delete(void* pointer);

public:

	MemoryFile(byte *buffer, int length);
	MemoryFile(AbstractFile *file);
	virtual ~MemoryFile();

	virtual bool  isOpen() const;
	virtual int   length() const;
	virtual int   tell() const;
	virtual bool  seek(SeekType seekType, int offset);
	virtual int   read(void *destinationBuffer, int numberOfBytes);
	virtual int   write(int numberOfBytes, const void *sourceBuffer);
	virtual void  close();

	virtual byte *readEntireFileAndClose();

private:

	MemoryFile();
	MemoryFile(const MemoryFile &);
	MemoryFile &operator =(const MemoryFile &);

	static void remove();

private:

	static MemoryBlockManager  *ms_memoryBlockManager;

private:

	byte                 *m_buffer;
	const int             m_length;
	int                   m_offset;
};

// ======================================================================

#endif
