// ======================================================================
//
// ZlibFile.cpp
// Copyright 2002, Sony Online Entertainment Inc.
// All Rights Reserved.
//
// ======================================================================

#include "sharedFile/FirstSharedFile.h"
#include "sharedFile/ZlibFile.h"

#include "sharedFoundation/ExitChain.h"
#include "sharedFoundation/MemoryBlockManager.h"
#include "sharedCompression/ZlibCompressor.h"
#include "sharedFile/MemoryFile.h"

// ======================================================================

MemoryBlockManager  *ZlibFile::ms_memoryBlockManager;

// ======================================================================

void ZlibFile::install()
{
	DEBUG_FATAL(ms_memoryBlockManager, ("ZlibFile::install already installed"));
	ms_memoryBlockManager = new MemoryBlockManager("ZlibFile::memoryBlockManager", true, sizeof(ZlibFile), 0, 0, 0);
	ExitChain::add(&remove, ("ZlibFile::remove"));
}

// ----------------------------------------------------------------------

void ZlibFile::remove()
{
	DEBUG_FATAL(!ms_memoryBlockManager,("ZlibFile is not installed"));

	delete ms_memoryBlockManager;
	ms_memoryBlockManager = 0;
}

// ----------------------------------------------------------------------

void *ZlibFile::operator new(size_t size)
{
	DEBUG_FATAL(!ms_memoryBlockManager,("ZlibFile is not installed"));

	// do not try to alloc a descendant class with this allocator
	DEBUG_FATAL(size != sizeof(ZlibFile),("bad size"));
	UNREF(size);

	return ms_memoryBlockManager->allocate();
}

// ----------------------------------------------------------------------

void  ZlibFile::operator delete(void *pointer)
{
	DEBUG_FATAL(!ms_memoryBlockManager,("ZlibFile is not installed"));
	ms_memoryBlockManager->free(pointer);
}

// ======================================================================

ZlibFile::ZlibFile(int uncompressedLength, byte *compressedBuffer, int compressedBufferLength, bool ownsCompressedBuffer)
: AbstractFile(PriorityData),
	m_uncompressedLength(uncompressedLength),
	m_compressedBuffer(compressedBuffer),
	m_compressedBufferLength(compressedBufferLength),
	m_ownsCompressedBuffer(ownsCompressedBuffer),
	m_decompressedMemoryFile(nullptr)
{
}

// ----------------------------------------------------------------------

ZlibFile::~ZlibFile()
{
	close();
}

// ----------------------------------------------------------------------

bool ZlibFile::isOpen() const
{
	return m_compressedBuffer || m_decompressedMemoryFile;
}

// ----------------------------------------------------------------------

int ZlibFile::length() const
{
	DEBUG_FATAL(!isOpen(), ("file is not open"));
	return m_uncompressedLength;
}

// ----------------------------------------------------------------------

int ZlibFile::tell() const
{
	DEBUG_FATAL(!isOpen(), ("file is not open"));
	if (!m_decompressedMemoryFile)
		return 0;

	return m_decompressedMemoryFile->tell();
}

// ----------------------------------------------------------------------

bool ZlibFile::seek(SeekType seekType, int offset)
{
	DEBUG_FATAL(!isOpen(), ("file is not open"));
	if (!m_decompressedMemoryFile)
		createDecompressedMemoryFile();
	return m_decompressedMemoryFile->seek(seekType, offset);
}

// ----------------------------------------------------------------------

int ZlibFile::read(void *destinationBuffer, int numberOfBytes)
{
	DEBUG_FATAL(!isOpen(), ("file is not open"));
	if (!m_decompressedMemoryFile)
		createDecompressedMemoryFile();
	return m_decompressedMemoryFile->read(destinationBuffer, numberOfBytes);
}

// ----------------------------------------------------------------------

int ZlibFile::write(int, const void *)
{
	DEBUG_FATAL(true, ("writing to a zlib file is not supported"));
	return 0;
}

// ----------------------------------------------------------------------

void ZlibFile::close()
{
	if (m_ownsCompressedBuffer)
		delete [] m_compressedBuffer;
	m_compressedBuffer = nullptr;

	delete m_decompressedMemoryFile;
	m_decompressedMemoryFile = nullptr;
}

// ----------------------------------------------------------------------

byte * ZlibFile::decompress() const
{
	byte * uncompressedBuffer = new byte[m_uncompressedLength];

	static_cast<void>(ZlibCompressor().expand(m_compressedBuffer, m_compressedBufferLength, uncompressedBuffer, m_uncompressedLength));

	return uncompressedBuffer;
}

// ----------------------------------------------------------------------

void ZlibFile::createDecompressedMemoryFile()
{
	MemoryFile * memoryFile = new MemoryFile(decompress(), m_uncompressedLength);
	close();
	m_decompressedMemoryFile = memoryFile;
}

// ----------------------------------------------------------------------

byte *ZlibFile::readEntireFileAndClose()
{
	DEBUG_FATAL(!isOpen(), ("file is not open"));
	byte *result = decompress();
	close();
	return result;
}

// ----------------------------------------------------------------------

bool ZlibFile::isZlibCompressed() const
{
	return true;
}

// ----------------------------------------------------------------------

int ZlibFile::getZlibCompressedLength() const
{
	return m_compressedBufferLength;
}

// ----------------------------------------------------------------------

void ZlibFile::getZlibCompressedDataAndClose(byte *& compressedBuffer, int & compressedBufferLength)
{
	DEBUG_FATAL(!isOpen(), ("file is not open"));
	DEBUG_FATAL(m_decompressedMemoryFile, ("cannot get compressed data after another non-compressed function has been used"));

	if (m_ownsCompressedBuffer)
	{
		compressedBuffer = m_compressedBuffer;
		m_compressedBuffer = nullptr;
		compressedBufferLength = m_compressedBufferLength;
	}
	else
	{
		compressedBuffer = new byte[m_compressedBufferLength];
		memcpy(compressedBuffer, m_compressedBuffer, m_compressedBufferLength);
		compressedBufferLength = m_compressedBufferLength;
	}

	close();
}

// ======================================================================
