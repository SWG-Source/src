// ======================================================================
//
// TailFileLogObserver.cpp
//
// Copyright 2002 Sony Online Entertainment
//
// ======================================================================

#include "sharedLog/FirstSharedLog.h"
#include "sharedFoundation/NetworkIdArchive.h"
#include "sharedLog/TailFileLogObserver.h"
#include "sharedLog/LogManager.h"
#include "fileInterface/StdioFile.h"
#include "sharedNetworkMessages/LogMessage.h"
#include "UnicodeUtils.h"
#include <string>
#include <cstdio>

// ======================================================================

const int LOG_FILESIZE_NEXT = 2000000000; // point at which we roll over to another log file
const int DEFAULT_NUM_LINES = 100;

namespace TailFileLogObserverNamespace
{
	std::vector<TailFileLogObserver *> s_tailFileLogObservers;
};

using namespace TailFileLogObserverNamespace;

// ======================================================================

void TailFileLogObserver::install()
{
	LogManager::registerObserverType("tailfile", create);
}

// ----------------------------------------------------------------------

LogObserver *TailFileLogObserver::create(std::string const &spec)
{
	return new TailFileLogObserver(spec);
}

// ----------------------------------------------------------------------

TailFileLogObserver::TailFileLogObserver(std::string const &filename) :
	LogObserver(),
	m_filename(filename),
	m_file(0),
	m_fileIndex(0),
	m_numLines(DEFAULT_NUM_LINES),
	m_textBuffer(0),
	m_nextTextBufferEntry(0)
{
	int filenameAsNumber = -1;
	filenameAsNumber = atoi(m_filename.c_str());
	m_numLines = filenameAsNumber;	
	std::string::size_type const index = filename.find_last_of(',');
	if(index != std::string::npos)
		m_filename = filename.substr(index + 1, filename.length());
	else
		m_filename.clear();
	m_textBuffer = new std::vector<std::string>(m_numLines);

	s_tailFileLogObservers.push_back(this);
}

// ----------------------------------------------------------------------
	
void TailFileLogObserver::setNumberOfLinesToKeep(int numLines)
{
	m_numLines = numLines;
	m_textBuffer->clear();
	m_nextTextBufferEntry = 0;
	m_textBuffer->reserve(m_numLines);
}

// ----------------------------------------------------------------------

void TailFileLogObserver::prepareFile()
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

TailFileLogObserver::~TailFileLogObserver()
{
	delete m_file;
	delete m_textBuffer;
}

// ----------------------------------------------------------------------

void TailFileLogObserver::log(LogMessage const &msg)
{
	std::string & newLogMessage = (*m_textBuffer)[m_nextTextBufferEntry++];

	uint64 timestamp           = msg.getTimestamp();
	std::string const &procId  = msg.getProcId();
	std::string const &channel = msg.getChannel();
	std::string const &text    = msg.getText();
	std::string uniAttach(Unicode::wideToNarrow(msg.getUnicodeAttach()));
	char tsbuf[16]; // yyyymmddhhmmss (14)

	newLogMessage.clear();
	IGNORE_RETURN( snprintf(tsbuf, 16, UINT64_FORMAT_SPECIFIER, timestamp) );	
	newLogMessage.append(tsbuf);
	newLogMessage.append(":");
	newLogMessage.append(procId);
	newLogMessage.append(":");
	newLogMessage.append(channel);
	newLogMessage.append(":");
	newLogMessage.append(text);
	if(uniAttach.length())
	{
		newLogMessage.append(":");
		newLogMessage.append(uniAttach);
	}
	
	if(m_nextTextBufferEntry >= m_numLines)
		m_nextTextBufferEntry = 0;	
}

// ----------------------------------------------------------------------

void TailFileLogObserver::flush(const char * filename)
{	
	if(m_filename.empty())
		m_filename = filename;
	prepareFile();
	NOT_NULL(m_file);
	
	int i;
	for(i = m_nextTextBufferEntry; i < m_numLines; ++i)
	{
		if(!(*m_textBuffer)[i].empty())
		{
			IGNORE_RETURN(m_file->write((*m_textBuffer)[i].length(), (*m_textBuffer)[i].c_str()) );
		}
	}	
	for(i = 0; i < m_nextTextBufferEntry; ++i)
	{
		if(!(*m_textBuffer)[i].empty())
		{
			IGNORE_RETURN(m_file->write((*m_textBuffer)[i].length(), (*m_textBuffer)[i].c_str()) );
		}
	}	
	if (m_file)
		m_file->flush();
	m_file->close();
}

// ----------------------------------------------------------------------

void TailFileLogObserver::flushAllTailFileLogObservers(const char * filename)
{
	for(std::vector<TailFileLogObserver *>::iterator i = s_tailFileLogObservers.begin(); i != s_tailFileLogObservers.end(); ++i)
	{
		TailFileLogObserver *tflo = *i;
		if(tflo)
		{
			tflo->flush(filename);
		}
	}
}

// ======================================================================

