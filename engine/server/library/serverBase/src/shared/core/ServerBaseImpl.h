//======================================================================
//
// ServerBaseImpl.h
// copyright (c) 2005 Sony Online Entertainment
// Author: Doug Mellencamp
//
//======================================================================

#ifndef INCLUDED_ServerBaseImpl_H
#define INCLUDED_ServerBaseImpl_H

//======================================================================

#include "serverBase/FirstServerBase.h"
#include "serverBase/ConfigServerBase.h"
#include "sharedNetwork/SetupSharedNetwork.h"
#include "sharedNetwork/NetworkHandler.h"
#include "sharedDebug/Profiler.h"
#include "sharedFoundation/Clock.h"
#include "sharedFoundation/Os.h"
#include "sharedFoundation/ExitChain.h"
#include "sharedFoundation/Timer.h"
#include "sharedLog/Log.h"
#include "sharedLog/LogManager.h"
#include "sharedLog/SetupSharedLog.h"

//-----------------------------------------------------------------------

class ServerBaseImpl
{
public:
    ServerBaseImpl();
	virtual ~ServerBaseImpl();

    // All implementations of ServerBase interface are here as public methods
public:

	virtual const uint32           getServerId                     ();
	virtual void                   setServerId                     (const uint32 id);
	virtual bool                   isDone                          ();
	virtual void                   setDone                         (const bool isDone);
	virtual void                   setSleepTimePerFrameMs          (int sleepTimePerFrameMs);
	virtual void                   run                             ();
	virtual void                   update                          (real time);
	virtual void                   setupConnections                ();
	virtual void                   unsetupConnections              ();
	virtual void                   preMainLoopInit                 ();
	virtual void                   postMainLoopCleanup             ();


// all implementation details are here as protected members, so subclasses have full access.
protected:

	uint32                         m_serverId;
	bool                           m_done;
	int                            m_sleepTimePerFrameMs;


	//----------------------------------------------------------------------

};


#endif

