// ======================================================================
//
// LogManager.h
//
// Copyright 2002 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_LogManager_H
#define INCLUDED_LogManager_H

// ======================================================================

class LogMessage;
class LogObserver;

// ======================================================================

// The LogManager is a static class which catches log messages,
// processes them from varArgs into straight ascii text, and passes the
// processed string on to any observers, along with a timestamp,
// process identifier, and any unicode text attached.

typedef LogObserver *(*LogObserverCreateFunc)(std::string const &);

class LogManager
{
public:

	static void install(std::string const &procId, bool flushOnWrite = true);
	static void update();
	static void setProcessIdentifier(std::string const &procId);

	// note: setArgs() and log() always come as a pair, and must do so to avoid deadlocks
	static void setArgs(std::string const &channel, Unicode::String const &unicodeAttach);
	static void setArgs(std::string const &channel);
	static void log(char const *format, ...);
	static void logLongText(std::string const & channel, std::string const & longText);

	static void observeLogMessage(LogMessage const &msg);

	static void addObserver(LogObserver *observer);
	static void removeObserver(LogObserver const *observer);
	static void registerObserverType(std::string const &name, LogObserverCreateFunc func);
	static bool setupObserver(std::string const &desc);
	static bool getFlushOnWrite();
	static void flush();
	
private:
	LogManager();
	LogManager(LogManager const &);
	LogManager &operator=(LogManager const &);
};

// ======================================================================

#endif

