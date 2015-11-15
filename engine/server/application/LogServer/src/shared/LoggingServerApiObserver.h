// LoggingServerApiObserver.h
// Copyright 2000-02, Sony Online Entertainment Inc., all rights reserved. 
// Author: Justin Randall

#ifndef	_INCLUDED_LoggingServerApiObserver_H
#define	_INCLUDED_LoggingServerApiObserver_H

//-----------------------------------------------------------------------

#include "sharedLog/LogObserver.h"

class LoggingServerApi;

//-----------------------------------------------------------------------

class LoggingServerApiObserver : public LogObserver
{
public:
	static void install();
	static LogObserver *create(std::string const &spec);

	LoggingServerApiObserver();
	virtual ~LoggingServerApiObserver();

	virtual void log(LogMessage const &msg);
	virtual void flush();
	virtual void update();

private:
	LoggingServerApiObserver & operator = (const LoggingServerApiObserver & rhs);
	LoggingServerApiObserver(const LoggingServerApiObserver & source);

	LoggingServerApi *  m_loggingServerApi;
};

//-----------------------------------------------------------------------

#endif	// _INCLUDED_LoggingServerApiObserver_H
