// ======================================================================
//
// FileLogObserver.cpp
//
// Copyright 2002 Sony Online Entertainment
//
// ======================================================================

#include "sharedLog/FirstSharedLog.h"
#include "sharedFoundation/NetworkIdArchive.h"
#include "sharedLog/FileLogObserver.h"
#include "sharedLog/LogManager.h"
#include "fileInterface/StdioFile.h"
#include "sharedNetworkMessages/LogMessage.h"
#include "UnicodeUtils.h"
#include <string>
#include <cstdio>

// ======================================================================

const int LOG_FILESIZE_NEXT = 2000000000; // point at which we roll over to another log file

// ======================================================================

void FileLogObserver::install()
{
	LogManager::registerObserverType("file", create);
}

// ----------------------------------------------------------------------

LogObserver *FileLogObserver::create(std::string const &spec)
{
	return new FileLogObserver(spec);
}

// ----------------------------------------------------------------------

FileLogObserver::FileLogObserver(std::string const &filename) :
	LogObserver(),
	m_filename(filename),
	m_file(0),
	m_fileIndex(0)
{
	prepareFile();
}

// ----------------------------------------------------------------------

void FileLogObserver::prepareFile()
{
	while (!m_file || m_file->tell() >= LOG_FILESIZE_NEXT)
	{
		if (m_file)
		{
			++m_fileIndex;
			delete m_file;
			m_file = 0;
		}
		if (m_fileIndex == 0)
			m_file = new StdioFile(m_filename.c_str(), "a");
		else
		{
			char buf[512];
			IGNORE_RETURN( snprintf(buf, 512, "%s-%d", m_filename.c_str(), m_fileIndex+1) );
			m_file = new StdioFile(buf, "a");
		}
		NOT_NULL(m_file);
		FATAL(!m_file->isOpen(), ("Could not open %s", m_filename.c_str()));
	}
}

// ----------------------------------------------------------------------

FileLogObserver::~FileLogObserver()
{
	delete m_file;
}

// ----------------------------------------------------------------------

void FileLogObserver::log(LogMessage const &msg)
{
	prepareFile();
	NOT_NULL(m_file);

	uint64 timestamp           = msg.getTimestamp();
	std::string const &procId  = msg.getProcId();
	std::string const &channel = msg.getChannel();
	std::string const &text    = msg.getText();
	std::string uniAttach(Unicode::wideToNarrow(msg.getUnicodeAttach()));
	char tsbuf[16]; // yyyymmddhhmmss (14)

	IGNORE_RETURN( snprintf(tsbuf, 16, UINT64_FORMAT_SPECIFIER, timestamp) );
	IGNORE_RETURN( m_file->write(14, tsbuf) );
	IGNORE_RETURN( m_file->write(1, ":") );
	IGNORE_RETURN( m_file->write(static_cast<int>(procId.length()), procId.c_str()) );
	IGNORE_RETURN( m_file->write(1, ":") );
	IGNORE_RETURN( m_file->write(static_cast<int>(channel.length()), channel.c_str()) );
	IGNORE_RETURN( m_file->write(1, ":") );
	IGNORE_RETURN( m_file->write(static_cast<int>(text.length()), text.c_str()) );
	if (uniAttach.length())
	{
		IGNORE_RETURN( m_file->write(1, ":") );
		IGNORE_RETURN( m_file->write(static_cast<int>(uniAttach.length()), uniAttach.c_str()) );
	}
	IGNORE_RETURN( m_file->write(1, "\n") );

	if (LogManager::getFlushOnWrite())
		m_file->flush();
}

// ----------------------------------------------------------------------

void FileLogObserver::flush()
{
	if (m_file)
		m_file->flush();
}

// ======================================================================

