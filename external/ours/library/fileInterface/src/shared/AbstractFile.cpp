// ======================================================================
//
// AbstractFile.cpp
// copyright (c) 2001,2002 Sony Online Entertainment
//
// ======================================================================

#include "fileInterface/FirstFileInterface.h"
#include "fileInterface/AbstractFile.h"

#include <assert.h>
#include <cstdio>

// ======================================================================

namespace AbstractFileNamespace
{
	AbstractFile::AudioServeFunction s_audioServeFunction = nullptr;
}

using namespace AbstractFileNamespace;

// ----------------------------------------------------------------------

void AbstractFile::setAudioServe(AudioServeFunction audioServeFunction)
{
	s_audioServeFunction = audioServeFunction;
}

// ======================================================================

AbstractFile::AbstractFile(PriorityType priority)
:
	m_priority(priority)
{
}

// ----------------------------------------------------------------------

AbstractFile::~AbstractFile()
{
}

// ----------------------------------------------------------------------

void AbstractFile::flush()
{
}

// ----------------------------------------------------------------------

byte *AbstractFile::readEntireFileAndClose()
{
	if (s_audioServeFunction != nullptr)
		(*s_audioServeFunction)();

	seek(SeekBegin, 0);

	const int fileLength = length();
	byte *buffer = new byte[fileLength];
	const int bytesRead = read(buffer, fileLength);
	static_cast<void>(bytesRead);
	assert(bytesRead == fileLength);
	close();

	return buffer;
}

// ----------------------------------------------------------------------

bool AbstractFile::isZlibCompressed() const
{
	return false;
}

// ----------------------------------------------------------------------

int AbstractFile::getZlibCompressedLength() const
{
	return -1;
}

// ----------------------------------------------------------------------

void AbstractFile::getZlibCompressedDataAndClose(byte *& compressedBuffer, int & compressedBufferLength)
{
	compressedBuffer = nullptr;
	compressedBufferLength = -1;
}

// ======================================================================

