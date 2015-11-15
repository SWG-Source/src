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

class SetupSharedFoundation
{
public:

	struct Data
	{
		typedef void (*LostFocusCallbackFuction)();

		// window creation stuff
		bool            createWindow;
		const char     *windowName;
		HICON           windowNormalIcon;
		HICON           windowSmallIcon;
		HINSTANCE       hInstance;

		// window use stuff
		bool            useWindowHandle;
		bool            processMessagePump;
		HWND            windowHandle;

		bool            writeMiniDumps;

		bool            clockUsesSleep;
		bool            clockUsesRecalibrationThread;

		// pointer to command line 
		const char     *commandLine;
		int             argc;
		char          **argv;

		// name of the config file to lead 
		const char     *configFile;

		// registry stuff
		const char     *productRegistryKey;

		real            frameRateLimit;


		bool            demoMode;

		bool            verboseWarnings;

		LostFocusCallbackFuction lostFocusCallback;

	public:

		enum Defaults
		{
			D_console,
			D_game,
			D_mfc
		};

		Data(Defaults defaults);

	private:

		Data();

		void setupGameDefaults();
		void setupConsoleDefaults();
		void setupMfcDefaults();
	};

public:

	typedef void (*MainFunction)();

public:

	static void install(const Data &data);
	static void remove(void);

	static void callbackWithExceptionHandling(MainFunction mainFunction);

private:

	SetupSharedFoundation();
	SetupSharedFoundation(const SetupSharedFoundation &);
	SetupSharedFoundation &operator =(const SetupSharedFoundation &);
};

// ======================================================================

#endif
