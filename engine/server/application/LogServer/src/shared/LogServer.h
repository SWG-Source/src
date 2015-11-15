// ======================================================================
//
// LogServer.h
//
// Copyright 2002 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_LogServer_H
#define INCLUDED_LogServer_H

// ======================================================================

#include "sharedMessageDispatch/Receiver.h"
#include "Singleton/Singleton.h"

// ======================================================================

class Service;

// ======================================================================

class LogServer: public Singleton<LogServer>, public MessageDispatch::Receiver
{
public:
	LogServer();
	~LogServer();

	static void install();
	static void remove();
	static void run();
	void mainLoop();
	void setDone(char const *reasonfmt, ...);

	virtual void receiveMessage(MessageDispatch::Emitter const &source, MessageDispatch::MessageBase const &message);

private:
	LogServer(LogServer const &);
	LogServer &operator=(LogServer const &);

	bool             m_done;
	Service *        m_logService;
};

// ======================================================================

#endif

