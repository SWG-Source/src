// ======================================================================
//
// ServerPathBuildManager.h
// Copyright 2001 Sony Online Entertainment Inc.
// All Rights Reserved.
//
// ======================================================================

#ifndef	INCLUDED_ServerPathBuildManager_H
#define	INCLUDED_ServerPathBuildManager_H

class ServerPathBuilder;

// ======================================================================

class ServerPathBuildManager
{
public:

	static void install        ( void );
	static void remove         ( void );

	static void update         ( float timeBudget );

	static bool queue          ( ServerPathBuilder * builder, bool highPriority );
	static bool unqueue        ( ServerPathBuilder * builder );
};

// ======================================================================

#endif // INCLUDED_ServerPathBuildManager_H

