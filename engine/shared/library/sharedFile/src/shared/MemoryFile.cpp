// ======================================================================
//
// MemoryFile.cpp
// Copyright 2002, Sony Online Entertainment Inc.
// All Rights Reserved.
//
// ======================================================================

#include "sharedFile/FirstSharedFile.h"
#include "sharedFile/MemoryFile.h"

#include "sharedFoundation/ExitChain.h"
#include "sharedFoundation/MemoryBlockManager.h"

// ======================================================================

MemoryBlockManager  *MemoryFile::ms_memoryBlockManager;

// ======================================================================

void MemoryFile::install()
{
	DEBUG_FATAL(ms_memoryBlockManager, ("MemoryFile::install already installed"));
	ms_memoryBlockManager = new MemoryBlockManager("MemoryFile::memoryBlockManager", true, sizeof(MemoryFile), 0, 0, 0);
	ExitChain::add(&remove, ("MemoryFile::remove"));
}

// ----------------------------------------------------------------------

void MemoryFile::remove()
{
	DEBUG_FATAL(!ms_memoryBlockManager,("MemoryFileis not installed"));

	delete ms_memoryBlockManager;
	ms_memoryBlockManager = 0;
}

// ----------------------------------------------------------------------

void *MemoryFile::operator new(size_t size)
{
	DEBUG_FATAL(!ms_memoryBlockManager,("MemoryFile is not installed"));

	// do not try to alloc a descendant class with this allocator
	DEBUG_FATAL(size != sizeof(MemoryFile),("bad size"));
	UNREF(size);

	return ms_memoryBlockManager->allocate();
}

// ----------------------------------------------------------------------

void  MemoryFile::operator delete(void *pointer)
{
	DEBUG_FATAL(!ms_memoryBlockManager,("Projectile is not installed"));
	ms_memoryBlockManager->free(pointer);
}

// ======================================================================

MemoryFile::MemoryFile(byte *buffer, int length)
: AbstractFile(PriorityData),
	m_buffer(buffer),
	m_length(length),
	m_offset(0)
{
}

// ----------------------------------------------------------------------

MemoryFile::MemoryFile(AbstractFile *file)
: AbstractFile(PriorityData),
	m_buffer(nullptr),
	m_length(file->length()),
	m_offset(0)
{
	m_buffer = file->readEntireFileAndClose();
}

// ----------------------------------------------------------------------

MemoryFile::~MemoryFile()
{
	close();
}

// ----------------------------------------------------------------------

bool MemoryFile::isOpen() const
{
	return m_buffer != nullptr;
}

// ----------------------------------------------------------------------

int MemoryFile::length() const
{
	DEBUG_FATAL(!isOpen(), ("file is not open"));
	return m_length;
}

// ----------------------------------------------------------------------

int MemoryFile::tell() const
{
	DEBUG_FATAL(!isOpen(), ("file is not open"));
	return m_offset;
}

// ----------------------------------------------------------------------

bool MemoryFile::seek(SeekType seekType, int offset)
{
	DEBUG_FATAL(!isOpen(), ("file is not open"));
	switch (seekType)
	{
		case SeekBegin:
			m_offset = offset;
			break;

		case SeekCurrent:
			m_offset += offset;
			break;

		case SeekEnd:
			m_offset = m_length + offset;
			break;
	}

	m_offset = clamp(0, m_offset, m_length);
	return true;
}

// ----------------------------------------------------------------------

int MemoryFile::read(void *destinationBuffer, int numberOfBytes)
{
	DEBUG_FATAL(!isOpen(), ("file is not open"));
	DEBUG_FATAL(numberOfBytes < 0,("asked to read %d bytes", numberOfBytes));

	// don't let them read past end-of-file
	if (m_offset + numberOfBytes > m_length)
		numberOfBytes = m_length - m_offset;

	// read from the buffer
	memcpy(destinationBuffer, m_buffer + m_offset, numberOfBytes);

	m_offset += numberOfBytes;
	return numberOfBytes;
}

// ----------------------------------------------------------------------

int MemoryFile::write(int, const void *)
{
	DEBUG_FATAL(true, ("writing to a memory file is not supported"));
	return 0;
}

// ----------------------------------------------------------------------

void MemoryFile::close()
{
	delete [] m_buffer;
	m_buffer = nullptr;
}

// ----------------------------------------------------------------------

byte *MemoryFile::readEntireFileAndClose()
{
	byte *result = m_buffer;
	m_buffer = nullptr;
	return result;
}

// ======================================================================
