// ======================================================================
//
// ZlibFile.h
// Copyright 2002, Sony Online Entertainment Inc.
// All Rights Reserved.
//
// ======================================================================

#ifndef INCLUDED_ZlibFile_H
#define INCLUDED_ZlibFile_H

// ======================================================================

class MemoryBlockManager;
class MemoryFile;

#include "fileInterface/AbstractFile.h"

// ======================================================================

class ZlibFile : public AbstractFile
{
public:

	static void           install();
	static void *operator new(size_t size);
	static void  operator delete(void* pointer);

public:

	ZlibFile(int uncompressedLength, byte *compressedBuffer, int compressedLength, bool ownsCompressedBuffer);
	virtual ~ZlibFile();

	virtual bool  isOpen() const;
	virtual int   length() const;
	virtual int   tell() const;
	virtual bool  seek(SeekType seekType, int offset);
	virtual int   read(void *destinationBuffer, int numberOfBytes);
	virtual int   write(int numberOfBytes, const void *sourceBuffer);
	virtual void  close();

	virtual byte *readEntireFileAndClose();

	virtual bool isZlibCompressed() const;
	virtual int  getZlibCompressedLength() const;
	virtual void getZlibCompressedDataAndClose(byte *& compressedBuffer, int & compressedBufferLength);

private:

	static void remove();

private:

	ZlibFile();
	ZlibFile(const ZlibFile &);
	ZlibFile &operator =(const ZlibFile &);

	byte * decompress() const;
	void createDecompressedMemoryFile();

private:

	static MemoryBlockManager  *ms_memoryBlockManager;

private:

	const int m_uncompressedLength;
	byte *m_compressedBuffer;
	const int m_compressedBufferLength;
	bool m_ownsCompressedBuffer;
	mutable MemoryFile *m_decompressedMemoryFile;
};

// ======================================================================

#endif
