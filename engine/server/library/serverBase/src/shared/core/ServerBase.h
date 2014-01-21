// ======================================================================
//
// ServerBase.h
// Copyright 2005, Sony Online Entertainment Inc., all rights reserved.
// Author: Doug Mellencamp
//
// ======================================================================

#ifndef INCLUDED_ServerBase_H
#define INCLUDED_ServerBase_H

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

//======================================================================

class ServerBaseImpl;

//----------------------------------------------------------------------

class ServerBase
{
public:

	static void                   install                         ();
	static void                   remove                          ();
	static const uint32           getServerId                     ();
	static void                   setServerId                     (const uint32 id);
	static bool                   isDone                          ();
	static void                   setDone                         (const bool isDone);
	static void                   setSleepTimePerFrameMs          (int sleepTimePerFrameMs);
	static void                   run                             ();
	static void                   update                          (real time);
	static void                   setupConnections                ();
	static void                   unsetupConnections              ();
	static void                   preMainLoopInit                 ();
	static void                   postMainLoopCleanup             ();

protected:

	static void                   install                         ( ServerBaseImpl *impl );

    static ServerBaseImpl*   s_implementation;
};




//======================================================================

#endif
