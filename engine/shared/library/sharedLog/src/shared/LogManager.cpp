// ======================================================================
//
// LogManager.cpp
//
// Copyright 2002 Sony Online Entertainment
//
// ======================================================================

#include "sharedLog/FirstSharedLog.h"
#include "sharedFoundation/NetworkIdArchive.h"

#include "sharedFoundation/ExitChain.h"
#include "sharedFoundation/ConfigFile.h"
#include "sharedLog/LogManager.h"
#include "sharedLog/LogObserver.h"
#include "sharedLog/ConfigSharedLog.h"
#include "sharedLog/StderrLogger.h"
#include "sharedNetworkMessages/LogMessage.h"
#include "sharedSynchronization/RecursiveMutex.h"
#include <list>
#include <map>
#include <string>
#include <cstdio>
#include <time.h>

// ======================================================================

typedef std::list<LogObserver *> ObserverList;
typedef std::map<std::string, LogObserverCreateFunc> ObserverCreateMap;

// ======================================================================

namespace LogManagerNamespace
{
	void remove();

	const int MaxLogMessageLen = 16384;

	struct LogManagerData
	{
		LogManagerData() :
			mutex(),
			observers(),
			observerCreateMap(),
			channel(),
			unicodeAttach(),
			processIdentifier(),
			flushOnWrite(true),
			logging(0)
		{
		}
		RecursiveMutex mutex;
		ObserverList observers;
		ObserverCreateMap observerCreateMap;
		std::string channel;
		Unicode::String unicodeAttach;
		std::string processIdentifier;
		bool flushOnWrite;
		int logging; // used for catching recursive log attempts from the same thread

	private:
		LogManagerData(LogManagerData const &);
		LogManagerData &operator=(LogManagerData const &);
	};
	LogManagerData *s_data;
}
using namespace LogManagerNamespace;

// ======================================================================

void LogManager::install(std::string const &procId, bool flushOnWrite)
{
	DEBUG_FATAL(s_data, ("LogManager already installed"));
	s_data = new LogManagerData;
	s_data->processIdentifier = procId;
	s_data->flushOnWrite = flushOnWrite;
	ExitChain::add(LogManagerNamespace::remove, "LogManagerNamespace::remove");
	if (ConfigSharedLog::getLogStderr())
		StderrLogger::install();
}

// ----------------------------------------------------------------------

void LogManager::update()
{
	if (!s_data)
		return;

	if (ConfigSharedLog::getLogStderr())
		StderrLogger::update();

	for (ObserverList::iterator i = s_data->observers.begin(); i != s_data->observers.end(); ++i)
		(*i)->update();
}

// ----------------------------------------------------------------------

void LogManagerNamespace::remove()
{
	DEBUG_FATAL(!s_data, ("LogManager not installed"));
	// we own any observers we have, so delete them
	while (!s_data->observers.empty())
	{
		delete (*s_data->observers.begin());
		IGNORE_RETURN( s_data->observers.erase(s_data->observers.begin()) );
	}
	if (ConfigSharedLog::getLogStderr())
		StderrLogger::remove();

	delete s_data;
	s_data = 0;
}

// ----------------------------------------------------------------------

void LogManager::setProcessIdentifier(std::string const &procId)
{
	s_data->processIdentifier = procId;
}

// ----------------------------------------------------------------------

void LogManager::setArgs(std::string const &channel, Unicode::String const &unicodeAttach)
{
	if (!s_data)
		return;

	s_data->mutex.enter(); // leave() called from log which should always follow setArgs
	if (s_data->logging == 0)
	{
		++s_data->logging;
		s_data->channel = channel;
		s_data->unicodeAttach = unicodeAttach;
	}
}

// ----------------------------------------------------------------------

void LogManager::setArgs(std::string const &channel)
{
	if (!s_data)
		return;

	s_data->mutex.enter(); // leave() called from log which should always follow setArgs
	++s_data->logging;
	if (s_data->logging == 1)
	{
		s_data->channel = channel;
		s_data->unicodeAttach.clear();
	}
}

// ----------------------------------------------------------------------

void LogManager::log(char const *format, ...)
{
	if (!s_data)
		return;
	// if noone is observing log messages, no need to process them
	if (s_data->observers.size() && s_data->logging == 1)
	{
		++s_data->logging;
		static char text[MaxLogMessageLen];
		{
			va_list ap;
			va_start(ap, format); //lint !e746 !e1055
			IGNORE_RETURN( _vsnprintf(text, MaxLogMessageLen, format, ap) );
			text[sizeof(text)-1] = '\0';
			size_t len = strlen(text);
			// if string was truncated, stick a + on the end
			if (len == sizeof(text)-1)
				text[len-1] = '+';
			va_end(ap);
		}

		uint64 timestamp;
		{
			// format current date/time gmt as yyyymmddhhmmss, in a uint64
			time_t now;
			tm t;

			IGNORE_RETURN( time(&now) );
			IGNORE_RETURN( gmtime_r(&now, &t) );
			timestamp = t.tm_year+1900; //lint !e732 !e737 !e776
			timestamp *= 100;
			timestamp += t.tm_mon+1; //lint !e737 !e776
			timestamp *= 100;
			timestamp += static_cast<unsigned int>(t.tm_mday);
			timestamp *= 100;
			timestamp += static_cast<unsigned int>(t.tm_hour);
			timestamp *= 100;
			timestamp += static_cast<unsigned int>(t.tm_min);
			timestamp *= 100;
			timestamp += static_cast<unsigned int>(t.tm_sec);
		}

		observeLogMessage(LogMessage(timestamp, s_data->processIdentifier, s_data->channel, text, s_data->unicodeAttach));
		--s_data->logging;
	}

	--s_data->logging;
	s_data->mutex.leave(); // enter() called from setArgs which should always precede log
}

// ----------------------------------------------------------------------

void LogManager::observeLogMessage(LogMessage const &msg)
{
	for (ObserverList::iterator i = s_data->observers.begin(); i != s_data->observers.end(); ++i)
		if (!(*i)->isFiltered(msg))
			(*i)->log(msg);
}

// ----------------------------------------------------------------------

void LogManager::addObserver(LogObserver *observer)
{
	s_data->observers.push_back(observer);
}

// ----------------------------------------------------------------------

void LogManager::removeObserver(LogObserver const *observer)
{
	// we own any observers given to us, so delete it when it is removed
	for (ObserverList::iterator i = s_data->observers.begin(); i != s_data->observers.end(); ++i)
	{
		if ((*i) == observer)
		{
			delete (*i);
			IGNORE_RETURN( s_data->observers.erase(i) );
			break;
		}
	}
}

// ----------------------------------------------------------------------

void LogManager::registerObserverType(std::string const &name, LogObserverCreateFunc func)
{
	s_data->observerCreateMap[name] = func;

	char buffer[256];
	snprintf(buffer, 256, "%s:", name.c_str());

	// run through SharedLog keys looking for logTarget=name:
	{
		int const numberLogTargets = ConfigSharedLog::getNumberOfLogTargets();
		for (int i = 0; i < numberLogTargets; ++i)
		{
			char const * const result = ConfigSharedLog::getLogTarget(i);
			if (result && strstr(result, buffer) == result)
			{
				setupObserver(result);
			}
		}
	}

	// run through SharedLog keys looking for logTarget#=name:
	{
		char key[16];
		strcpy(key, "logTarget");

		for (int i = 0; i < 20; ++i)
		{
			char const * result = 0;
			snprintf(key+9, 25, "%d", i);
			int count = 0;
			do
			{
				result = ConfigFile::getKeyString("SharedLog", key, count, 0);
				if (result && strstr(result, buffer) == result)
					setupObserver(result);
				++count;
			} while (result);
		}
	}
}

// ----------------------------------------------------------------------

bool LogManager::setupObserver(std::string const &desc)
{
	REPORT_LOG(true, ("Log observer setup: %s\n", desc.c_str()));

	// description should be "type:spec{filter1:filter2:...}"
	size_t pos = desc.find(":");
	if (pos != std::string::npos)
	{
		std::string type(desc.substr(0, pos));
		std::string spec;
		std::string filt;

		size_t pos2 = desc.find("{");
		if (pos2 == std::string::npos)
			spec = desc.substr(pos+1);
		else
		{
			spec = desc.substr(pos+1, (pos2-(pos+1)));
			filt = desc.substr(pos2+1);
			if (filt.length() && filt[filt.length()-1] == '}')
			{
				IGNORE_RETURN( filt.erase(filt.length()-1) );
			}
		}

		ObserverCreateMap::iterator i = s_data->observerCreateMap.find(type);
		if (i != s_data->observerCreateMap.end())
		{
			LogObserver *observer = (*(*i).second)(spec);
			if (observer)
			{
				if (filt.length())
					observer->setFilter(filt);
				addObserver(observer);
				return true;
			}
		}
	}
	return false;
}

// ----------------------------------------------------------------------

bool LogManager::getFlushOnWrite()
{
	return s_data->flushOnWrite;
}

// ----------------------------------------------------------------------

void LogManager::flush()
{
	for (ObserverList::iterator i = s_data->observers.begin(); i != s_data->observers.end(); ++i)
		(*i)->flush();
}

// ----------------------------------------------------------------------

void LogManager::logLongText(std::string const & channel, std::string const & longText)
{
	int const targetLineLength=80;
	int const maxLineLength=100;
	char buffer[maxLineLength+1];
	int pos=0;
	for (std::string::const_iterator i=longText.begin(); i!=longText.end(); ++i)
	{
		buffer[pos++]=*i;
		if ((pos>=targetLineLength && *i==' ') || (pos==maxLineLength))
		{
			buffer[pos]='\0';
			setArgs(channel);
			log("%s", buffer);
			pos=0;
		}
	}
	if (pos!=0)
	{
		buffer[pos]='\0';
		setArgs(channel);
		log("%s", buffer);
		pos=0;
	}
}

// ======================================================================

