// ======================================================================
//
// DatabaseLogObserver.h
//
// Copyright 2002 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_DatabaseLogObserver_H
#define INCLUDED_DatabaseLogObserver_H

// ======================================================================

#include "sharedLog/LogObserver.h"

// ======================================================================

// A DatabaseLogObserver filters relevant messages and saves them as
// appropriate to a database.
class DatabaseLogObserver: public LogObserver
{
public:
	static void install();
	static LogObserver *create(std::string const &spec);

	DatabaseLogObserver();
	virtual ~DatabaseLogObserver();

	virtual void log(LogMessage const &msg);

private:
	DatabaseLogObserver(DatabaseLogObserver const &);
	DatabaseLogObserver &operator=(DatabaseLogObserver const &);
};

// ======================================================================

#endif // INCLUDED_DatabaseLogObserver_H

