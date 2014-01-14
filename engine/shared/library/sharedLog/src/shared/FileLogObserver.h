// ======================================================================
//
// FileLogObserver.h
//
// Copyright 2002 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_FileLogObserver_H
#define INCLUDED_FileLogObserver_H

// ======================================================================

#include "sharedLog/LogObserver.h"
#include <string>

// ======================================================================

class AbstractFile;

// ======================================================================

// A FileLogObserver saves log messages to a file
class FileLogObserver: public LogObserver
{
public:

	static void install();
	static LogObserver *create(std::string const &spec);

	FileLogObserver(std::string const &filename);
	virtual ~FileLogObserver();

	virtual void log(LogMessage const &msg);
	virtual void flush();

private:
	FileLogObserver(FileLogObserver const &);
	FileLogObserver &operator=(FileLogObserver const &);

	void prepareFile();

private:
	std::string m_filename;
	AbstractFile *m_file;
	int m_fileIndex;
};

// ======================================================================

#endif

