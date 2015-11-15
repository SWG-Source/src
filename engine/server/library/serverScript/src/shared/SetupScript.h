// ======================================================================
//
// SetupScript.h
// copyright 1998 Bootprint Entertainment
//
// ======================================================================

#ifndef INCLUDED_SETUPSCRIPT_H
#define INCLUDED_SETUPSCRIPT_H

// ======================================================================

class SetupScript
{
private:

	// **** INCREMENT THIS VERSION NUMBER WHENEVER THIS STRUCT CHANAGES ****

	// This enum is private so that a game can't use it directly.  If the
	// struct changes, the game programmers should be forced to look at the
	// changes and update their data and version number manually.

	enum
	{
		DATA_VERSION = 0
	};

public:

	struct Data
	{
		int             version;

            // script data
 		bool            useRemoteDebugJava;	// flag to use the jvm for remote debugging
    };

public:

	static void setupDefaultGameData(Data &data);

	static void install();
	static void remove(void);
};

// ======================================================================

#endif
