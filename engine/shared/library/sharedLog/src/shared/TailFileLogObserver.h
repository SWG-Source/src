// ======================================================================
//
// TailFileLogObserver.h
//
// Copyright 2002 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_TailFileLogObserver_H
#define INCLUDED_TailFileLogObserver_H

// ======================================================================

#include "sharedLog/LogObserver.h"
#include <string>

// ======================================================================

class AbstractFile;

// ======================================================================

// A TailFileLogObserver saves log messages to a file, but only the last N,
// and only when requested (not constantly).
class TailFileLogObserver: public LogObserver
{
public:

	static void install();
	static LogObserver *create(std::string const &spec);

	TailFileLogObserver(std::string const &filename);
	virtual ~TailFileLogObserver();

	virtual void log(LogMessage const &msg);
	virtual void flush(const char * filename);

	virtual void setNumberOfLinesToKeep(int numLines);

	static void flushAllTailFileLogObservers(const char *filename);

private:
	TailFileLogObserver(TailFileLogObserver const &);
	TailFileLogObserver &operator=(TailFileLogObserver const &);

	void prepareFile();

private:
	std::string m_filename;
	AbstractFile *m_file;
	int m_fileIndex;

	int m_numLines;
	std::vector<std::string> *m_textBuffer;
	int m_nextTextBufferEntry;
};

// ======================================================================

#endif

