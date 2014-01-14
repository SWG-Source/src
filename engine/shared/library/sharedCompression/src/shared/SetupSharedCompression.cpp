// ======================================================================
//
// SetupSharedCompression.cpp
// Copyright 2003, Sony Online Entertainment Inc.
// All Rights Reserved.
//
// ======================================================================

#include "sharedCompression/FirstSharedCompression.h"
#include "sharedCompression/SetupSharedCompression.h"

#include "sharedCompression/ZlibCompressor.h"
#include "sharedDebug/InstallTimer.h"

// ======================================================================

SetupSharedCompression::Data::Data()
:
	numberOfThreadsAccessingZlib(1)
{
}

// ======================================================================

void SetupSharedCompression::install()
{
	InstallTimer const installTimer("SetupSharedCompression::install");

	Data data;
	install(data);
}

// ----------------------------------------------------------------------

void SetupSharedCompression::install(Data const &data)
{
	ZlibCompressor::install(data.numberOfThreadsAccessingZlib);
}

// ======================================================================
