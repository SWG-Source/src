//
// ConfigServerscript.h
// asommers 9-11-2000
//
// copyright 2000, verant interactive
//

#ifndef _INCLUDED_ConfigServerScript_H
#define _INCLUDED_ConfigServerScript_H


//-------------------------------------------------------------------

class ConfigServerScript
{
  public:

    struct Data
    {
        // script data
        const char *    scriptPath;         // location of scripts
        const char *    javaLibPath;        // location of jvm files
        const char *	javaDebugPort;		// ip port we connect to for remote debugging
	bool            useRemoteDebugJava;	// flag to use the jvm for remote debugging
	bool            allowBuildClusterScriptAttach; //used only on the build cluster for attaching scripts without a VM.
	bool            allowDebugConsoleMessages; //used only on the build cluster for attaching scripts without a VM.
	bool            allowDebugSpeakMessages;   //used to enable/disable the debugSpeakMessage script function
	bool            disableScriptLogs;
	bool            logBalance;
	bool		printStacks;
    };

  private:

    static Data *data;

  public:

    static void install						(void);
    static void remove						(void);

    static const char *     getScriptPath               (void);
    static const char *     getJavaLibPath              (void);
    static const char *     getJavaDebugPort            (void);
    static bool             getUseRemoteDebugJava       (void);
    static bool             getAllowBuildClusterScriptAttach();
    static bool             allowDebugConsoleMessages();
    static bool             allowDebugSpeakMessages();
    static bool             getDisableScriptLogs();
    static bool             getLogBalance();

    static bool		    getPrintStacks();

    static bool hasJavaOptions();
    static int getNumberOfJavaOptions();
    static char const * getJavaOptions(int index);
};

inline const char *ConfigServerScript::getScriptPath(void)
{
	return data->scriptPath;
}

//-----------------------------------------------------------------------

inline const char *ConfigServerScript::getJavaLibPath(void)
{
	return data->javaLibPath;
}

//-----------------------------------------------------------------------

inline const char *ConfigServerScript::getJavaDebugPort(void)
{
	return data->javaDebugPort;
}

//-----------------------------------------------------------------------

inline bool ConfigServerScript::getUseRemoteDebugJava(void)
{
	return data->useRemoteDebugJava;
}

//-----------------------------------------------------------------------

inline bool ConfigServerScript::getAllowBuildClusterScriptAttach()
{
	return data->allowBuildClusterScriptAttach;
}

//-----------------------------------------------------------------------

inline bool ConfigServerScript::allowDebugConsoleMessages()
{
	return data->allowDebugConsoleMessages;
}

inline bool ConfigServerScript::allowDebugSpeakMessages()
{
	return data->allowDebugSpeakMessages;
}

//-----------------------------------------------------------------------

inline bool ConfigServerScript::getDisableScriptLogs()
{
	return data->disableScriptLogs;
}

//-----------------------------------------------------------------------

inline bool ConfigServerScript::getLogBalance()
{
	return data->logBalance;
}

inline bool ConfigServerScript::getPrintStacks()
{
#ifdef _DEBUG
	return true;
#else
	return data->printStacks;
#endif
}

//-----------------------------------------------------------------------


#endif	// _INCLUDED_ConfigServerScript_H

