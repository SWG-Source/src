//
// ConfigServerScript.cpp
//
// copyright 2000, verant interactive
//
#include "serverScript/FirstServerScript.h"
#include "serverScript/ConfigServerScript.h"
#include "sharedMath/Vector.h"

#include "sharedFoundation/ConfigFile.h"

#include <vector>

//-------------------------------------------------------------------

ConfigServerScript::Data* ConfigServerScript::data;

//-------------------------------------------------------------------

static ConfigServerScript::Data staticData;

//-------------------------------------------------------------------
//lint -e750  Disabling un used macros warning
#define KEY_INT(a,b)    (data->a = ConfigFile::getKeyInt("ServerScript", #a, b))
#define KEY_BOOL(a,b)   (data->a = ConfigFile::getKeyBool("ServerScript", #a, b))
#define KEY_REAL(a,b)   (data->a = ConfigFile::getKeyReal("ServerScript", #a, b))
#define KEY_STRING(a,b) (data->a = ConfigFile::getKeyString("ServerScript", #a, b))

//-------------------------------------------------------------------

namespace ConfigServerScriptNamespace
{
	typedef std::vector<char const *> StringPtrArray;
	StringPtrArray ms_javaOptions; // ConfigFile owns the pointer
}

using namespace ConfigServerScriptNamespace;

// ======================================================================

bool ConfigServerScript::hasJavaOptions()
{
	return !ms_javaOptions.empty();
}

// ----------------------------------------------------------------------

int ConfigServerScript::getNumberOfJavaOptions()
{
	return static_cast<int>(ms_javaOptions.size());
}

// ----------------------------------------------------------------------

char const * ConfigServerScript::getJavaOptions(int index)
{
	VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE(0, index, getNumberOfJavaOptions());
	return ms_javaOptions[static_cast<size_t>(index)];
}

// ----------------------------------------------------------------------

void ConfigServerScript::install(void)
{
	data = &staticData;

	KEY_STRING  (scriptPath, "../../data/sku.0/sys.server/compiled/game/script");
	KEY_STRING  (javaLibPath, "./jvm");
	KEY_STRING  (javaDebugPort, "8000");
	KEY_BOOL    (useRemoteDebugJava, false);
	KEY_BOOL    (allowBuildClusterScriptAttach, false);
	KEY_BOOL    (allowDebugConsoleMessages, true);
	KEY_BOOL    (allowDebugSpeakMessages, true);
	KEY_BOOL    (disableScriptLogs, false);
	KEY_BOOL    (logBalance, false);
	KEY_BOOL    (printStacks, false);

	int index = 0;
	char const * result = 0;
	do
	{
		// "javaOptions" are in the "GameServer" block and not the "ServerScript" block
		result = ConfigFile::getKeyString("GameServer", "javaOptions", index++, 0);
		if (result != 0)
		{
			ms_javaOptions.push_back(result);
		}
	}
	while (result);
}

//-------------------------------------------------------------------

void ConfigServerScript::remove(void)
{
}

//-------------------------------------------------------------------



