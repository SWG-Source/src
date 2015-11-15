// ======================================================================
//
// SetupSharedFoundation.h
// copyright 1998 Bootprint Entertainment
// copyright 2001 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_SetupSharedFoundation_H
#define INCLUDED_SetupSharedFoundation_H

// ======================================================================

struct DebugMenuEntry;

// ======================================================================

class SetupSharedFoundation
{
public:

	struct Data
	{
		// allow running in background
		bool            runInBackground;

		// pointer to command line 
		char*           lpCmdLine;
		int             argc;
		char          **argv;

		const char     *configFile;
		
		real            frameRateLimit;

	public:

		enum Defaults
		{
			D_game,
			D_console
		};

		Data(Defaults defaults);
	};

public:

	static void install(const Data &data);
	static void remove(void);

	static void callbackWithExceptionHandling(void (*callback)(void));

};

// ======================================================================

#endif
