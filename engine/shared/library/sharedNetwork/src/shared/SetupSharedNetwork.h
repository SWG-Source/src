// ======================================================================
//
// SetupSharedNetwork.h
// Copyright 2003 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#ifndef INCLUDED_SetupSharedNetwork_H
#define INCLUDED_SetupSharedNetwork_H

// ======================================================================

class SetupSharedNetwork
{
public:

	struct SetupData
	{
		int  m_clockSyncDelay;
	};

public:

	static void getDefaultClientSetupData(SetupData &data);
	static void getDefaultServerSetupData(SetupData &data);

	static void install(SetupData &setupData);

};

// ======================================================================

#endif
