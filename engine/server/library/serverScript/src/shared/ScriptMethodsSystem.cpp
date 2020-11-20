// ScriptMethodsSystem.cpp - implements script methods that are stubs needing real implementation
//
// copyright 2002 Sony Online Entertainment
//
//========================================================================

#include "serverScript/FirstServerScript.h"
#include "serverScript/JavaLibrary.h"

#include "serverGame/BuildingObject.h"
#include "serverGame/Client.h"
#include "serverGame/ServerObject.h"
#include "sharedFoundation/ConfigFile.h"
#include "sharedNetworkMessages/ConsoleChannelMessages.h"
#include "sharedNetworkMessages/GenericValueTypeMessage.h"
#include "sharedNetworkMessages/PlayCutSceneMessage.h"


// ======================================================================
// ScriptMethodsSystemNamespace
// ======================================================================

namespace ScriptMethodsSystemNamespace
{
	bool install();

	jboolean     JNICALL sendConsoleCommand(JNIEnv * env, jobject self, jstring command, jlong target);
	jstring      JNICALL getConfigSetting(JNIEnv * env, jobject self, jstring section, jstring key);
	jboolean     JNICALL fixHouseItemLimit(JNIEnv * env, jobject self, jlong target);
	void         JNICALL saveTextOnClient(JNIEnv * env, jobject self, jlong jclient, jstring jfilename, jstring jfiletext);
	void         JNICALL saveBytesOnClient(JNIEnv * env, jobject self, jlong jclient, jstring jfilename, jbyteArray bytes);
	void         JNICALL launchClientWebBrowser(JNIEnv *env, jobject self, jlong player, jstring url);
	void         JNICALL playCutScene(JNIEnv * env, jobject self, jlong jclient, jstring jfilename);
	void		 JNICALL triggerServerWarning(JNIEnv * env, jobject self, jstring message);
}


//========================================================================
// install
//========================================================================

bool ScriptMethodsSystemNamespace::install()
{
const JNINativeMethod NATIVES[] = {
	#define JF(a,b,c) {a,b,(void*)(ScriptMethodsSystemNamespace::c)}
	JF("_sendConsoleCommand", "(Ljava/lang/String;J)Z", sendConsoleCommand),
	JF("getConfigSetting", "(Ljava/lang/String;Ljava/lang/String;)Ljava/lang/String;", getConfigSetting),
	JF("_fixHouseItemLimit", "(J)Z", fixHouseItemLimit),
	JF("_saveTextOnClient", "(JLjava/lang/String;Ljava/lang/String;)V", saveTextOnClient),
	JF("_saveBytesOnClient", "(JLjava/lang/String;[B)V", saveBytesOnClient),
	JF("_launchClientWebBrowser", "(JLjava/lang/String;)V", launchClientWebBrowser),
	JF("_playCutScene", "(JLjava/lang/String;)V", playCutScene),
	JF("_triggerServerWarning", "(Ljava/lang/String;)V", triggerServerWarning),
};

	return JavaLibrary::registerNatives(NATIVES, sizeof(NATIVES)/sizeof(NATIVES[0]));
}


// ----------------------------------------------------------------------

jboolean JNICALL ScriptMethodsSystemNamespace::sendConsoleCommand(JNIEnv * env, jobject self, jstring command, jlong target)
{
	JavaStringParam localCommand(command);

	ServerObject* object = nullptr;
	if (!JavaLibrary::getObject(target, object))
		return JNI_FALSE;

	Client* client = object->getClient();
	if (!client)
		return JNI_FALSE;

	std::string commandName;
	JavaLibrary::convert(localCommand, commandName);

	ExecuteConsoleCommand msg(commandName);
	client->send(msg,true);
	return JNI_TRUE;
}

// ----------------------------------------------------------------------
/**
 * Returns the value of a config file key.
 *
 * @param env		    Java environment
 * @param self		    class calling this function
 * @param section		the config file section
 * @param key			the config file key
 *
 * @return the key value, or nullptr if the key doesn't exist
 */
jstring JNICALL ScriptMethodsSystemNamespace::getConfigSetting(JNIEnv * env, jobject self,
	jstring section, jstring key)
{
	UNREF(self);

	if (section == 0 || key == 0)
		return 0;

	JavaStringParam jsection(section);
	JavaStringParam jkey(key);

	std::string sectionName;
	if (!JavaLibrary::convert(jsection, sectionName))
		return 0;

	std::string keyName;
	if (!JavaLibrary::convert(jkey, keyName))
		return 0;

	const ConfigFile::Section * sec = ConfigFile::getSection(sectionName.c_str());
	if (sec == nullptr)
		return 0;

	const ConfigFile::Key * ky = sec->findKey(keyName.c_str());
	if (ky == nullptr)
		return nullptr;

	JavaString jvalue(ky->getAsString(ky->getCount()-1, ""));
	return jvalue.getReturnValue();
}	// JavaLibrary::getConfigSetting

// ----------------------------------------------------------------------
/**
 * Utility function to fix a bug with houses.
 *
 * @param env		    Java environment
 * @param self		    class calling this function
 * @param target		the house to fix
 *
 * @return true on success, false if target isn't a building
 */
jboolean JNICALL ScriptMethodsSystemNamespace::fixHouseItemLimit(JNIEnv * env, jobject self, jlong target)
{
	UNREF(self);

	TangibleObject *pob = 0;
	if (!JavaLibrary::getObject(target, pob) || !pob->getPortalProperty())
		return JNI_FALSE;

	pob->fixUpPobItemLimit();
	return JNI_TRUE;
}	// JavaLibrary::fixHouseItemLimit

// ----------------------------------------------------------------------

void JNICALL ScriptMethodsSystemNamespace::saveTextOnClient(JNIEnv *, jobject, jlong jclient, jstring jfilename, jstring jfiletext)
{
	ServerObject *obj = 0;
	if (JavaLibrary::getObject(jclient, obj))
	{
		Client * const client = obj->getClient();
		if (client && client->isGod())
		{
			JavaStringParam filenameParam(jfilename);
			JavaStringParam filetextParam(jfiletext);
			std::string filename, filetext;
			if (JavaLibrary::convert(filenameParam, filename) && JavaLibrary::convert(filetextParam, filetext))
			{
				GenericValueTypeMessage<std::pair<std::string, std::string> > const msg("SaveTextOnClient", std::make_pair(filename, filetext));
				client->send(msg, true);
			}
		}
	}
}

// ----------------------------------------------------------------------

void JNICALL ScriptMethodsSystemNamespace::saveBytesOnClient(JNIEnv *, jobject, jlong jclient, jstring jfilename, jbyteArray bytes)
{
	ServerObject *obj = 0;
	if (JavaLibrary::getObject(jclient, obj))
	{
		Client * const client = obj->getClient();
		if (client && client->isGod())
		{
			JavaStringParam filenameParam(jfilename);
			std::string filename;
			if (JavaLibrary::convert(filenameParam, filename))
			{
				if (bytes != 0)
				{
					std::vector<int8> bytesStr;
					if (ScriptConversion::convert(bytes, bytesStr))
					{
						GenericValueTypeMessage<std::pair<std::string, std::string> > const msg("SaveTextOnClient", std::make_pair(filename, std::string(bytesStr.begin(), bytesStr.end())));
						client->send(msg, true);
					}
				}
			}
		}
	}
}

// ----------------------------------------------------------------------

void JNICALL ScriptMethodsSystemNamespace::launchClientWebBrowser(JNIEnv *env, jobject self, jlong player, jstring url)
{
	ServerObject const *  obj = 0;
	if (JavaLibrary::getObject(player, obj))
	{
		Client const * const client = obj->getClient();
		if (client)
		{
			JavaStringParam urlParam(url);
			std::string urlString;
			if (JavaLibrary::convert(urlParam, urlString))
				client->launchWebBrowser(urlString);
			else
				JAVA_THROW_SCRIPT_EXCEPTION(true, ("launchClientWebBrowser called with a url parameter that could not be converted to a string (network id was %s)",obj->getNetworkId().getValueString().c_str()));
		}
	}
	else
		JAVA_THROW_SCRIPT_EXCEPTION(true, ("launchClientWebBrowser was called with a network id paramter that could not be converted to an object."));
}

// ----------------------------------------------------------------------

void JNICALL ScriptMethodsSystemNamespace::playCutScene(JNIEnv * env, jobject self, jlong jclient, jstring jfilename)
{
	ServerObject *obj = 0;
	if (JavaLibrary::getObject(jclient, obj))
	{
		Client * const client = obj->getClient();
		if (client)
		{
			JavaStringParam filenameParam(jfilename);
			std::string fileName;
			if(JavaLibrary::convert(filenameParam, fileName))
			{
				PlayCutSceneMessage cutScene(fileName);
				client->send(cutScene, true);
			}
		}
	}
}

// ----------------------------------------------------------------------

void JNICALL ScriptMethodsSystemNamespace::triggerServerWarning(JNIEnv * env, jobject self, jstring message)
{
	JavaStringParam localMessage(message);
	std::string messageString;
	JavaLibrary::convert(localMessage, messageString);
	WARNING(true, (messageString.c_str()));
}

// ======================================================================

