//======================================================================
//
// ServerBaseImpl.cpp
// copyright (c) 2005 Sony Online Entertainment
// Author: Doug Mellencamp
//
//======================================================================

#include "serverBase/ServerBaseImpl.h"

//======================================================================

namespace ServerBaseImplNamespace
{


}

using namespace ServerBaseImplNamespace;

//======================================================================

ServerBaseImpl::ServerBaseImpl() :
		m_serverId(0),
		m_done(false),
    	m_sleepTimePerFrameMs(0)
{

}

//----------------------------------------------------------------------

ServerBaseImpl::~ServerBaseImpl()
{

}


//-----------------------------------------------------------------------


const uint32 ServerBaseImpl::getServerId( void )
{
	return m_serverId;
}

//-----------------------------------------------------------------------

void ServerBaseImpl::setServerId( const uint32 id )
{
	m_serverId = id;
}

//-----------------------------------------------------------------------

void ServerBaseImpl::setDone(const bool done)
{
	m_done = done;
}

//-----------------------------------------------------------------------

bool ServerBaseImpl::isDone()
{
	return m_done;
}

//-----------------------------------------------------------------------

void ServerBaseImpl::run()
{
	LOG("ServerStartup",("%s starting on %s", ConfigServerBase::getServerName(), NetworkHandler::getHostName().c_str()));  // no process id yet

	REPORT_LOG(true, ("\t[ServerBase] : ServerBaseImpl::run() - setting up connections.\n"));
	setupConnections();

	REPORT_LOG(true, ("\t[ServerBase] : ServerBaseImpl::run() - Performing pre main loop initializations.\n"));
	preMainLoopInit();

	REPORT_LOG(true, ("\t[ServerBase] : ServerBaseImpl::run() - Entering main loop.\n"));
	PROFILER_BLOCK_DEFINE(profileBlockMainLoop, "main loop");
	PROFILER_BLOCK_ENTER(profileBlockMainLoop);

	while (!isDone())
	{
		NetworkHandler::update();
		NetworkHandler::dispatch();
		update(time(0));
		NetworkHandler::clearBytesThisFrame();
		Os::sleep(m_sleepTimePerFrameMs);
	}

	PROFILER_BLOCK_LEAVE(profileBlockMainLoop);

	REPORT_LOG(true, ("\t[ServerBase] : ServerBaseImpl::run() - Performing post main loop cleanup.\n"));
	postMainLoopCleanup();

	REPORT_LOG(true, ("\t[ServerBase] : ServerBaseImpl::run() - un setting up connections.\n"));
	unsetupConnections();

	LOG("ServerStartup",("%s %lu exiting", ConfigServerBase::getServerName(), Os::getProcessId()));

}

//-----------------------------------------------------------------------

void ServerBaseImpl::setupConnections()
{
}

//-----------------------------------------------------------------------

void ServerBaseImpl::unsetupConnections ()
{
}

//-----------------------------------------------------------------------

void ServerBaseImpl::preMainLoopInit()
{
}

//-----------------------------------------------------------------------

void ServerBaseImpl::postMainLoopCleanup ()
{
}

//-----------------------------------------------------------------------

void ServerBaseImpl::update(real time)
{
}

//-----------------------------------------------------------------------
void ServerBaseImpl::setSleepTimePerFrameMs(int sleepTimePerFrameMs)
{
	m_sleepTimePerFrameMs = sleepTimePerFrameMs;
}

//======================================================================


