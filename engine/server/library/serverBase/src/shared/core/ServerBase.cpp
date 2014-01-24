// ======================================================================
//
// ServerBase.cpp
// Copyright 2005, Sony Online Entertainment Inc., all rights reserved.
// Author: Doug Mellencamp
//
// ======================================================================

#include "serverBase/ServerBase.h"

#include "ServerBaseImpl.h"

//======================================================================

ServerBaseImpl *ServerBase::s_implementation = 0;


//----------------------------------------------------------------------

void ServerBase::install ()
{
	install(new ServerBaseImpl());
}

//----------------------------------------------------------------------

void ServerBase::install (ServerBaseImpl* implementation)
{
	REPORT_LOG(true, ("\t[serverBase] : ServerBase::install() - begin\n"));

	DEBUG_FATAL (s_implementation, ("ServerBase already installed"));
	s_implementation = implementation;

	ConfigServerBase::install();
	SetupSharedNetwork::SetupData  networkSetupData;
	SetupSharedNetwork::getDefaultServerSetupData(networkSetupData);
	SetupSharedNetwork::install(networkSetupData);
#ifndef WIN32
	Os::setProgramName(ConfigServerBase::getServerName());
#endif
	NetworkHandler::install();
	char tmp[128] = {"\0"};
	IGNORE_RETURN(snprintf(tmp, sizeof(tmp), "%s:%d", ConfigServerBase::getServerName() ,Os::getProcessId()));
	SetupSharedLog::install(tmp);
	
	ExitChain::add(remove, "ServerBase::remove");

	REPORT_LOG(true, ("\t[serverBase] : ServerBase::install() - end\n"));
}

//----------------------------------------------------------------------

void ServerBase::remove  ()
{
    DEBUG_FATAL( !s_implementation, ("ServerBase implementation not set"));

	REPORT_LOG(true, ("\t[serverBase] : ServerBase::remove.\n"));

	SetupSharedLog::remove();
	NetworkHandler::remove();
	ConfigServerBase::remove();

    delete s_implementation;
    s_implementation = 0;
}


//----------------------------------------------------------------------

const uint32 ServerBase::getServerId()
{
    DEBUG_FATAL( !s_implementation, ("ServerBase implementation not set"));
    return (s_implementation->getServerId());
}

//----------------------------------------------------------------------

void ServerBase::setServerId(const uint32 id)
{
    DEBUG_FATAL( !s_implementation, ("ServerBase implementation not set"));
    s_implementation->setServerId(id);
}

//----------------------------------------------------------------------

bool ServerBase::isDone()
{
    DEBUG_FATAL( !s_implementation, ("ServerBase implementation not set"));
    return(s_implementation->isDone());
}

//----------------------------------------------------------------------

void ServerBase::setDone(const bool isDone)
{
    DEBUG_FATAL( !s_implementation, ("ServerBase implementation not set"));
    s_implementation->setDone(isDone);
}

//----------------------------------------------------------------------

void ServerBase::setSleepTimePerFrameMs(int sleepTimePerFrameMs)
{
    DEBUG_FATAL( !s_implementation, ("ServerBase implementation not set"));
    s_implementation->setSleepTimePerFrameMs(sleepTimePerFrameMs);
}

//----------------------------------------------------------------------

void ServerBase::run()
{
    DEBUG_FATAL( !s_implementation, ("ServerBase implementation not set"));
    s_implementation->run();
}

//----------------------------------------------------------------------

void ServerBase::update(real time)
{
    DEBUG_FATAL( !s_implementation, ("ServerBase implementation not set"));
    s_implementation->update(time);
}

//----------------------------------------------------------------------

void ServerBase::setupConnections()
{
    DEBUG_FATAL( !s_implementation, ("ServerBase implementation not set"));
    s_implementation->setupConnections();
}

//----------------------------------------------------------------------

void ServerBase::unsetupConnections()
{
    DEBUG_FATAL( !s_implementation, ("ServerBase implementation not set"));
    s_implementation->unsetupConnections();
}

//----------------------------------------------------------------------

void ServerBase::preMainLoopInit()
{
    DEBUG_FATAL( !s_implementation, ("ServerBase implementation not set"));
    s_implementation->preMainLoopInit();
}

//----------------------------------------------------------------------

void ServerBase::postMainLoopCleanup()
{
    DEBUG_FATAL( !s_implementation, ("ServerBase implementation not set"));
    s_implementation->postMainLoopCleanup();
}

//======================================================================


