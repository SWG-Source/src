// ======================================================================
//
// LogServer.cpp
//
// Copyright 2002 Sony Online Entertainment
//
// ======================================================================

#include "FirstLogServer.h"
#include "LogServer.h"

#include "ConfigLogServer.h"
#include "LoggerConnection.h"
#include "DatabaseLogObserver.h"
#include "LoggingServerApiObserver.h"
#include "sharedFoundation/Clock.h"
#include "sharedFoundation/Os.h"
#include "sharedLog/Log.h"
#include "sharedLog/SetupSharedLog.h"
#include "sharedNetwork/NetworkSetupData.h"
#include "sharedNetwork/Service.h"
#include "sharedNetworkMessages/GameNetworkMessage.h"
#include "sharedNetworkMessages/LogMessage.h"
#include "Archive/ByteStream.h"

#include <cstdio>

// ======================================================================

LogServer::LogServer() :
	Singleton<LogServer>(),
	MessageDispatch::Receiver(),
	m_done(false),
	m_logService(0)
{
	NetworkSetupData setup;
	setup.port = ConfigLogServer::getLogServicePort();
	setup.maxConnections = ConfigLogServer::getLogServiceMaxConnections();
	setup.bindInterface = ConfigLogServer::getLogServiceBindInterface();

	// set up the log service
	m_logService = new Service(ConnectionAllocator<LoggerConnection>(), setup);
	connectToMessage("LogMessage");
}

// ----------------------------------------------------------------------

LogServer::~LogServer()
{
	if (m_logService)
	{
		delete m_logService;
		m_logService = 0;
	}
}

// ----------------------------------------------------------------------

void LogServer::install()
{
	NetworkHandler::install();
	ConfigLogServer::install();

	// tell the logger not to flush on writes when installing
	SetupSharedLog::install("LogServer", false);
	DatabaseLogObserver::install();
	LoggingServerApiObserver::install();
}

// ----------------------------------------------------------------------

void LogServer::remove()
{
	SetupSharedLog::remove();
	ConfigLogServer::remove();
	NetworkHandler::remove();
}

// ----------------------------------------------------------------------

void LogServer::run()
{
	LogServer::getInstance().mainLoop();
}

// ----------------------------------------------------------------------

void LogServer::mainLoop()
{
	unsigned long limit = 100;
	unsigned long startTime;

	while (!m_done)
	{
		if (!Os::update())
			setDone("Os condition (Parent pid change)");

		startTime = Clock::timeMs();

		do
		{
			NetworkHandler::dispatch();
			LogManager::flush();
			Os::sleep(1);
			NetworkHandler::update();
		} while (Clock::timeMs() - startTime < limit);
	}

	// prevent shutdown issues from LogServer destructor running after LogServer::remove
	delete m_logService;
	m_logService = 0;
}

// ----------------------------------------------------------------------

void LogServer::receiveMessage(MessageDispatch::Emitter const &source, MessageDispatch::MessageBase const &message)
{
	UNREF(source);
	if (message.isType("LogMessage"))
	{
		Archive::ReadIterator ri = static_cast<GameNetworkMessage const &>(message).getByteStream().begin();
		// Pass the text to the log manager to be observed - it has already been
		// timestamped, etc.
		LogManager::observeLogMessage(LogMessage(ri));
	}
}

// ----------------------------------------------------------------------

void LogServer::setDone(char const *reasonfmt, ...)
{
	if (!m_done)
	{
		char reason[1024];
		va_list ap;
		va_start(ap, reasonfmt);
		_vsnprintf(reason, sizeof(reason), reasonfmt, ap);
		reason[sizeof(reason)-1] = '\0';

		LOG(
			"ServerShutdown",
			(
				"LogServer (pid %d) shutdown, reason: %s",
				static_cast<int>(Os::getProcessId()),
				reason));

		REPORT_LOG(
			true,
			(
				"LogServer (pid %d) shutdown, reason: %s\n",
				static_cast<int>(Os::getProcessId()),
				reason));

		va_end(ap);
		m_done = true;
	}

#ifdef ENABLE_PROFILING
	std::exit(0);
#endif
}

// ======================================================================

