// ======================================================================
//
// LogObserver.h
//
// Copyright 2002 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_LogObserver_H
#define INCLUDED_LogObserver_H

// ======================================================================

class LogMessage;

// ======================================================================

// A LogObserver is derived from in order to provide a mechanism for
// catching log messages and doing something with them.  A derived
// observer object must be added to the logManager before it will
// have any effect.

class LogObserver
{
public:
	LogObserver();
	virtual ~LogObserver();

	virtual void log(LogMessage const &msg) = 0;
	virtual void flush();

	void setFilter(std::string const &filter);
	bool isFiltered(LogMessage const &msg) const;

	virtual void update();
	
private:
	LogObserver(LogObserver const &);
	LogObserver &operator=(LogObserver const &);

	std::vector<std::string>* m_filters;
};

// ======================================================================

#endif

