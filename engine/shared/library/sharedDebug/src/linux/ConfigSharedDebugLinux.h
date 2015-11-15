// ======================================================================
//
// ConfigSharedDebugLinux.h
// Copyright 2002 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#ifndef INCLUDED_ConfigSharedDebugLinux_H
#define INCLUDED_ConfigSharedDebugLinux_H

//===================================================================

class ConfigSharedDebugLinux
{
public:

	static void  install();

	static bool        getUseTty();
	static char const *getDebugMonitorOutputFilename();
	static bool        getLogTtySetup();

};

//===================================================================

#endif

