// ======================================================================
//
// FileStreamerFile.cpp
// Copyright 2002, Sony Online Entertainment Inc.
// All Rights Reserved.
//
// ======================================================================

#include "sharedFile/FirstSharedFile.h"
#include "sharedFile/FileStreamerFile.h"

#include "sharedFile/OsFile.h"
#include "sharedFile/FileStreamer.h"
#include "sharedFoundation/ExitChain.h"
#include "sharedFoundation/MemoryBlockManager.h"

// ======================================================================

MemoryBlockManager *FileStreamerFile::ms_memoryBlockManager;

// ======================================================================

void FileStreamerFile::install()
{
	DEBUG_FATAL(ms_memoryBlockManager, ("FileStreamerFile::install already installed"));
	ms_memoryBlockManager = new MemoryBlockManager("FileStreamerFile::memoryBlockManager", true, sizeof(FileStreamerFile), 0, 0, 0);
	ExitChain::add(&FileStreamerFile::remove, "FileStreamerFile::remove");
}

// ----------------------------------------------------------------------

void FileStreamerFile::remove()
{
	DEBUG_FATAL(!ms_memoryBlockManager,("FileStreamerFile is not installed"));

	delete ms_memoryBlockManager;
	ms_memoryBlockManager = 0;
}

// ----------------------------------------------------------------------

void *FileStreamerFile::operator new(size_t size)
{
	DEBUG_FATAL(!ms_memoryBlockManager,("FileStreamerFile is not installed"));

	// do not try to alloc a descendant class with this allocator
	DEBUG_FATAL(size != sizeof(FileStreamerFile),("bad size"));
	UNREF(size);

	return ms_memoryBlockManager->allocate();
}

// ----------------------------------------------------------------------

void  FileStreamerFile::operator delete(void *pointer)
{
	DEBUG_FATAL(!ms_memoryBlockManager,("Projectile is not installed"));
	ms_memoryBlockManager->free(pointer);
}

// ======================================================================

FileStreamerFile::FileStreamerFile(PriorityType priority, FileStreamer::File &file)
: AbstractFile(priority),
	m_file(&file),
	m_owner(true),
	m_baseOffset(0),
	m_length(m_file->length()),
	m_offset(0)
{
}

// ----------------------------------------------------------------------

FileStreamerFile::FileStreamerFile(PriorityType priority, FileStreamer::File &file, int baseOffset, int length)
: AbstractFile(priority),
	m_file(&file),
	m_owner(false),
	m_baseOffset(baseOffset),
	m_length(length),
	m_offset(0)
{
}

// ----------------------------------------------------------------------

FileStreamerFile::~FileStreamerFile()
{
	close();
}

// ----------------------------------------------------------------------

bool FileStreamerFile::isOpen() const
{
	return m_file != nullptr;
}

// ----------------------------------------------------------------------

int FileStreamerFile::length() const
{
	DEBUG_FATAL(!isOpen(), ("file not open"));
	return m_length;
}

// ----------------------------------------------------------------------

int FileStreamerFile::tell() const
{
	DEBUG_FATAL(!isOpen(), ("file not open"));
	return m_offset;
}

// ----------------------------------------------------------------------

bool FileStreamerFile::seek(SeekType seekType, int offset)
{
	DEBUG_FATAL(!isOpen(), ("file not open"));

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

int FileStreamerFile::read(void *destinationBuffer, int numberOfBytes)
{
	DEBUG_FATAL(!isOpen(), ("file not open"));

	// make sure they don't read more file 
	if (m_offset + numberOfBytes > m_length)
		numberOfBytes = m_length - m_offset;

	const int bytesRead = m_file->read(m_baseOffset + m_offset, destinationBuffer, numberOfBytes, m_priority);
	m_offset += bytesRead;
	return bytesRead;
}

// ----------------------------------------------------------------------

int FileStreamerFile::write(int, const void *)
{
	DEBUG_FATAL(true, ("cannot call write on tree files"));
	return 0;
}

// ----------------------------------------------------------------------

void FileStreamerFile::flush()
{
	DEBUG_FATAL(true, ("cannot call flush on tree files"));
}

// ----------------------------------------------------------------------

void FileStreamerFile::close()
{
	if (m_owner)
		delete m_file;
	m_file = nullptr;
}

// ======================================================================
