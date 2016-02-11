// ======================================================================
//
// StdioFile.cpp
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#include "fileInterface/FirstFileInterface.h"
#include "fileInterface/StdioFile.h"

#include <cstdio>
#include <assert.h>
#include <string.h>

// ======================================================================

StdioFile::StdioFile(const char *fileName, const char *openType)
: AbstractFile(AbstractFile::PriorityData),
  m_file(fopen(fileName, openType)),
	m_justWrote(false)
{
}

// ----------------------------------------------------------------------

StdioFile::~StdioFile()
{
	close();
}

// ----------------------------------------------------------------------

void StdioFile::close()
{
	if(m_file)
	{
		fclose(m_file);
		m_file = nullptr;
	}
}

// ----------------------------------------------------------------------

bool StdioFile::isOpen() const
{
	return m_file != nullptr;
}

// ----------------------------------------------------------------------
// This routine is not exactly bitwise const but is sematically const in the end
int StdioFile::length() const
{
	const int oldPosition = tell ();
	const_cast<StdioFile*>(this)->seek (AbstractFile::SeekEnd, 0);
	const int fileLength = tell ();
	const_cast<StdioFile*>(this)->seek (AbstractFile::SeekBegin, oldPosition);
	return fileLength;
}

// ----------------------------------------------------------------------

int StdioFile::tell() const
{
	if (!m_file)
		return 0;

	return static_cast<int>(ftell(m_file));
}

// ----------------------------------------------------------------------

bool StdioFile::seek(SeekType seekType, int offset)
{
	assert(m_file != nullptr);

	m_justWrote = false;
	int result = 0;
	switch(seekType)
	{
		case SeekBegin:
			result = fseek(m_file, offset, SEEK_SET);
			break;

		case SeekCurrent:
			result = fseek(m_file, offset, SEEK_CUR);
			break;

		case SeekEnd:
			result = fseek(m_file, offset, SEEK_END);
			break;
	}

	return (result == 0);
}

// ----------------------------------------------------------------------

int StdioFile::read(void* dest_buffer, int num_bytes)
{
	assert(m_file != nullptr);
	resyncStream();
	return static_cast<int>(fread(dest_buffer, 1, static_cast<unsigned int>(num_bytes), m_file));
}

// ----------------------------------------------------------------------

int StdioFile::write(int num_bytes, const void* source_buffer)
{
	assert(m_file != nullptr);
	m_justWrote = true;
	return static_cast<int>(fwrite(source_buffer, 1, static_cast<unsigned int>(num_bytes), m_file));
}

// ----------------------------------------------------------------------

void StdioFile::flush()
{
	assert(m_file != nullptr);
	fflush(m_file);
	m_justWrote = false;
}

// ----------------------------------------------------------------------

void StdioFile::resyncStream()
{
	if (m_justWrote)
	{
		fseek(m_file, 0, SEEK_CUR);
		m_justWrote = false;
	}
}

// ================================================================

AbstractFile* StdioFileFactory::createFile(const char *fileName, const char *openType)
{
	if(!fileName || !openType)
		return nullptr;
	else if (fileName[0] == '\0' || openType[0] == '\0')
		return nullptr;
	else
		return new StdioFile(fileName, openType);
}

// ================================================================
