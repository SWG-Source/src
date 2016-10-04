// LoggingServerApiObserver.cpp
// Copyright 2000-02, Sony Online Entertainment Inc., all rights reserved.
// Author: Justin Randall

//-----------------------------------------------------------------------

#include "FirstLogServer.h"
#include "ConfigLogServer.h"
#include "LoggingServerApiObserver.h"
#include "LoggingServerApi.h"
#include "sharedLog/LogManager.h"
#include "sharedNetworkMessages/LogMessage.h"
#include "UnicodeUtils.h"

//-----------------------------------------------------------------------

LoggingServerApiObserver::LoggingServerApiObserver() :
	m_loggingServerApi(new LoggingServerApi(0, ConfigLogServer::getLoggingServerApiQueueSize()))
{
	m_loggingServerApi->Connect(ConfigLogServer::getLoggingServerApiAddress(), LoggingServerApi::cDefaultPort, ConfigLogServer::getLoggingServerApiLoginName(), ConfigLogServer::getLoggingServerApiPassword(), ConfigLogServer::getLoggingServerApiDefaultDirectory());
}

//-----------------------------------------------------------------------

LoggingServerApiObserver::~LoggingServerApiObserver()
{
	flush();
	delete m_loggingServerApi;
}

//-----------------------------------------------------------------------

void LoggingServerApiObserver::install()
{
	LogManager::registerObserverType("LoggingServerApi", create);
}

//-----------------------------------------------------------------------

LogObserver * LoggingServerApiObserver::create(const std::string &)
{
	return new LoggingServerApiObserver();
}

//-----------------------------------------------------------------------

void LoggingServerApiObserver::log(const LogMessage & msg)
{
	std::string fileName = "misc.txt";

	std::string msgText;

	if (!msg.getText().empty())
	{
		time_t now;
		tm t;

		IGNORE_RETURN(time(&now));
		IGNORE_RETURN(gmtime_r(&now, &t));
		char dirBuf[128] = { "\0" };
		snprintf(dirBuf, 128, "%d/%d/%d/", t.tm_year + 1900, t.tm_mon + 1, t.tm_mday);
		fileName = ConfigLogServer::getClusterName();
		fileName += "/";
		fileName += dirBuf;

		std::string splittext = msg.getText();
		if (splittext.find(":") != splittext.npos)
		{
			fileName += splittext.substr(0, splittext.find(":")) + ".txt";
		}

		msgText = splittext.substr(splittext.find(":") + 1);

		// If there is ANY Unicode, send the whole log as Unicode. If there is ONLY non-Unicode, then send the log non-Unicode.

		if (!msg.getUnicodeAttach().empty())
		{
			Unicode::String text;

			if (!msgText.empty())
			{
				text += Unicode::narrowToWide(msgText);
				text += Unicode::narrowToWide(":");
			}

			text += msg.getUnicodeAttach().c_str();
			text += Unicode::narrowToWide("\0");

			m_loggingServerApi->Log16(fileName.c_str(), 0, text.c_str());
		}
		else if (!msgText.empty())
		{
			m_loggingServerApi->Log(fileName.c_str(), 0, "%s\0", msgText.c_str());
		}
	}
}

//-----------------------------------------------------------------------

void LoggingServerApiObserver::flush()
{
	m_loggingServerApi->Flush(1000);
}

//-----------------------------------------------------------------------

void LoggingServerApiObserver::update()
{
	if (m_loggingServerApi->GetStatus() == LoggingServerApi::cStatusDisconnected)
	{
		m_loggingServerApi->Connect(ConfigLogServer::getLoggingServerApiAddress(), LoggingServerApi::cDefaultPort, ConfigLogServer::getLoggingServerApiLoginName(), ConfigLogServer::getLoggingServerApiPassword(), ConfigLogServer::getLoggingServerApiDefaultDirectory());
	}

	m_loggingServerApi->GiveTime();
}

//-----------------------------------------------------------------------