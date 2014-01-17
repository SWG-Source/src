// ======================================================================
//
// DatabaseLogObserver.cpp
//
// Copyright 2002 Sony Online Entertainment
//
// ======================================================================

#include "FirstLogServer.h"
#include "DatabaseLogObserver.h"

#include "sharedLog/LogManager.h"
#include <string>

// ======================================================================

void DatabaseLogObserver::install()
{
	LogManager::registerObserverType("db", create);
}

// ----------------------------------------------------------------------

LogObserver *DatabaseLogObserver::create(std::string const &spec)
{
	UNREF(spec);
	return new DatabaseLogObserver();
}

// ----------------------------------------------------------------------

DatabaseLogObserver::DatabaseLogObserver() :
	LogObserver()
{
}

// ----------------------------------------------------------------------

DatabaseLogObserver::~DatabaseLogObserver()
{
}

// ----------------------------------------------------------------------

void DatabaseLogObserver::log(LogMessage const &)
{
	// TODO - implement this
}

// ======================================================================

