//========================================================================
//
// ScriptMethodsNpc.cpp - implements script methods dealing with Npcs.
//
// copyright 2001 Sony Online Entertainment
//
//========================================================================

#include "serverScript/FirstServerScript.h"
#include "serverScript/JavaLibrary.h"

#include "serverGame/ConfigServerGame.h"
#include "serverGame/CreatureObject.h"
#include "serverGame/GameServer.h"
#include "serverGame/NameManager.h"
#include "serverGame/ServerCreatureObjectTemplate.h"
#include "sharedGame/OutOfBand.h"
#include "sharedGame/OutOfBandPackager.h"
#include "sharedGame/ProsePackage.h"
#include "sharedFoundation/Crc.h"
#include "sharedObject/ObjectTemplateList.h"
#include "UnicodeUtils.h"

using namespace JNIWrappersNamespace;


//========================================================================
// NPC conversation utility functions
//========================================================================

namespace ScriptMethodsNpcNamespace
{
	ProsePackage getProseFromOob (const Unicode::String & oob)
	{
		ProsePackage localPp;
		const OutOfBandBase * const base = OutOfBandPackager::unpackOne (oob, OutOfBandPackager::OT_prosePackage);
		if (base)
		{
			const OutOfBand <ProsePackage> * const oobpp = safe_cast<const OutOfBand <ProsePackage> *>(base);
			const ProsePackage * const pp = oobpp->getObject ();

			localPp = *pp;

			delete pp;
			delete base;
		}

		return localPp;
	}

	char const * const cms_conversationAppearanceOverride = "convo.appearance";

	bool install();
	void setNpcConversationResponses(JNIEnv *env, TangibleObject * player, jobjectArray responses);

	jboolean     JNICALL _npcStartConversation(JNIEnv *env, jobject self, jlong player, jlong npc, jstring convoName, jobject greeting, jstring greetingOob, jobjectArray responses, uint32 appearanceOverrideSharedTemplateCrc);
	jboolean     JNICALL npcStartConversation(JNIEnv *env, jobject self, jlong player, jlong npc, jstring convoName, jobject greeting, jstring greetingOob, jobjectArray responses);
	jboolean     JNICALL npcStartConversationAppearanceOverride(JNIEnv *env, jobject self, jlong player, jlong npc, jstring convoName, jobject greeting, jstring greetingOob, jobjectArray responses, jstring appearanceOverrideSharedTemplate);
	jboolean     JNICALL npcEndConversation(JNIEnv *env, jobject self, jlong player);
	jboolean     JNICALL npcEndConversationWithMessage(JNIEnv *env, jobject self, jlong player, jobject response, jstring responeOob);
	jboolean     JNICALL npcSpeak(JNIEnv *env, jobject self, jlong player, jobject text, jstring oob);
	jboolean     JNICALL npcSetConversationResponses(JNIEnv *env, jobject self, jlong player, jobjectArray responses);
	jboolean     JNICALL npcAddConversationResponse(JNIEnv *env, jobject self, jlong player, jobject response, jstring responeOob);
	jboolean     JNICALL npcRemoveConversationResponse(JNIEnv *env, jobject self, jlong player, jobject response);
	jboolean     JNICALL isInNpcConversation(JNIEnv *env, jobject self, jlong creature);
	jlongArray   JNICALL getNpcConversants(JNIEnv *env, jobject self, jlong creature);
	jstring      JNICALL generateRandomNameByTemplate(JNIEnv *env, jobject self, jstring templateName);
	jstring      JNICALL generateRandomNameByTable(JNIEnv *env, jobject self, jstring directory, jstring table);
	jboolean     JNICALL isNameReserved(JNIEnv *env, jobject self, jstring name);
	jboolean     JNICALL isNameReservedIgnoreRules(JNIEnv *env, jobject self, jstring name, jobjectArray ignoreRules);
	jboolean     JNICALL setNpcDifficulty(JNIEnv *env, jobject self, jlong npc, jlong difficulty);
}

using namespace ScriptMethodsNpcNamespace;


//========================================================================
// install
//========================================================================

bool ScriptMethodsNpcNamespace::install()
{
const JNINativeMethod NATIVES[] = {
	#define JF(a,b,c) {a,b,(void*)(ScriptMethodsNpcNamespace::c)}
	JF("__npcStartConversation",          "(JJLjava/lang/String;Lscript/string_id;Ljava/lang/String;[Ljava/lang/Object;)Z", npcStartConversation),
	JF("__npcStartConversationWithOverrideAppearance", "(JJLjava/lang/String;Lscript/string_id;Ljava/lang/String;[Ljava/lang/Object;Ljava/lang/String;)Z", npcStartConversationAppearanceOverride),
	JF("_npcEndConversation",             "(J)Z",                                       npcEndConversation),
	JF("_npcEndConversationWithMessage",  "(JLscript/string_id;Ljava/lang/String;)Z",   npcEndConversationWithMessage),
	JF("__npcSpeak",                      "(JLscript/string_id;Ljava/lang/String;)Z",   npcSpeak),
	JF("__npcSetConversationResponses",   "(J[Ljava/lang/Object;)Z",                    npcSetConversationResponses),
	JF("__npcAddConversationResponse",    "(JLscript/string_id;Ljava/lang/String;)Z",   npcAddConversationResponse),
	JF("_npcRemoveConversationResponse",  "(JLscript/string_id;)Z",                     npcRemoveConversationResponse),
	JF("_isInNpcConversation",            "(J)Z",                                       isInNpcConversation	),
	JF("_getNpcConversants",              "(J)[J",                        getNpcConversants),
	JF("generateRandomName",            "(Ljava/lang/String;)Ljava/lang/String;",                   generateRandomNameByTemplate),
	JF("generateRandomName",            "(Ljava/lang/String;Ljava/lang/String;)Ljava/lang/String;", generateRandomNameByTable),
	JF("isNameReserved",                "(Ljava/lang/String;)Z", isNameReserved),
	JF("isNameReserved",                "(Ljava/lang/String;[Ljava/lang/String;)Z", isNameReservedIgnoreRules),
	JF("_setNpcDifficulty",             "(JJ)Z",                                    setNpcDifficulty),
};

	return JavaLibrary::registerNatives(NATIVES, sizeof(NATIVES)/sizeof(NATIVES[0]));
}


//----------------------------------------------------------------------

/**
 * Takes a Java string_id array and converts it into npc conversation reponses
 * available to a player.
 *
 * @param env		    Java environment
 * @param player		the player
 * @param reponses		array of string_ids
*/
void ScriptMethodsNpcNamespace::setNpcConversationResponses(JNIEnv *env, TangibleObject * player, jobjectArray responses)
{
	StringId        stringId;
	Unicode::String oob;

	player->clearNpcConversation();

	if (responses == 0)
		return;

	const jsize count = env->GetArrayLength(responses);
	for (jsize i = 0; i < count; ++i)
	{
		LocalRefPtr response = getObjectArrayElement(LocalObjectArrayRefParam(responses), i);
		if (response != LocalRef::cms_nullPtr)
		{
			if (ScriptConversion::convert(*response, stringId))
			{
				// add the response to the player's conversation
				player->addNpcConversationResponse(stringId, ProsePackage ());
			}
			else if (isInstanceOf(*response, JavaLibrary::getClsString()))
			{
				oob.clear();
				jstring jstr = static_cast<jstring>(response->getValue());
				if (JavaLibrary::convert(JavaStringParam (jstr), oob))
				{
					const ProsePackage & pp = getProseFromOob (oob);
					if (pp.isValid ())
						player->addNpcConversationResponse (StringId (), pp);
					else
						WARNING (true, ("ScriptMethodsNpc setNpcConversationResponses error getting prose from oob index %i", i));
				}
				else
					WARNING (true, ("ScriptMethodsNpc setNpcConversationResponses could not convert jstring to Unicode::String"));
			}
			else
			{
				WARNING (true, ("ScriptMethodsNpc setNpcConversationResponses error in setNpcConversationResponses index %d", i));
			}
		}
		else
		{
			DEBUG_WARNING(true, ("NPC Conversation setNpcConversationResponses  response #%d (base-0) missing!", i));
		}
	}
}


//========================================================================
// class JavaLibrary JNI NPC callback methods
//========================================================================

//----------------------------------------------------------------------

/**
 * Starts a conversation between an npc and a player.
 *
 * @param env		    Java environment
 * @param self		    class calling this function
 * @param player		player in conversation
 * @param npc			npc in conversation
 * @param convoName		name of the conversation; passed to the trigger OnNpcConversationResponse
 * @param greeting		initial text npc will say to the player
 * @param responses		initial responses available to the player
 *
 * @return JNI_TRUE on success, JNI_FALSE on fail
 */
jboolean JNICALL ScriptMethodsNpcNamespace::_npcStartConversation(JNIEnv *env, jobject self, jlong player, jlong npc, jstring convoName, jobject greeting, jstring greetingOob, jobjectArray responses, uint32 appearanceOverrideTemplateCrc)
{
	UNREF(self);

	JavaStringParam localConvoName(convoName);
	std::string convoNameStr;
	if (!JavaLibrary::convert (localConvoName, convoNameStr))
		return JNI_FALSE;

	TangibleObject * playerObject = nullptr;
	if (!JavaLibrary::getObject(player, playerObject))
		return JNI_FALSE;

	if (playerObject->isInNpcConversation())
		return JNI_FALSE;

	TangibleObject * npcObject = nullptr;
	if (!JavaLibrary::getObject(npc, npcObject))
		return JNI_FALSE;

	if (!playerObject->startNpcConversation(*npcObject, convoNameStr, NpcConversationData::CS_NPC, appearanceOverrideTemplateCrc))
		return JNI_FALSE;

	// send the greeting
	StringId greetingStringId;
	if (greeting)
		ScriptConversion::convert(greeting, greetingStringId);

	Unicode::String oob;

	if (greetingOob)
	{
		JavaStringParam localGreetingOob(greetingOob);
		if (!JavaLibrary::convert (localGreetingOob, oob))
		{
			WARNING (true, ("ScriptMethodsNpc unable to convert greetingOob"));
			return JNI_FALSE;
		}
	}

	playerObject->sendNpcConversationMessage(greetingStringId, ProsePackage (), oob);

	// send the responses to the player
	setNpcConversationResponses(env, playerObject, responses);
	playerObject->sendNpcConversationResponses();

	return JNI_TRUE;
}	// JavaLibrary::NpcStartConversation

//----------------------------------------------------------------------

jboolean JNICALL ScriptMethodsNpcNamespace::npcStartConversation(JNIEnv *env, jobject self, jlong player, jlong npc, jstring convoName, jobject greeting, jstring greetingOob, jobjectArray responses)
{
	TangibleObject * speakerObject = nullptr;
	if (!JavaLibrary::getObject(npc, speakerObject) || !speakerObject)
		return JNI_FALSE;

	std::string conversationAppearanceOverride;
	speakerObject->getObjVars().getItem(cms_conversationAppearanceOverride, conversationAppearanceOverride);

	uint32 crc = Crc::crcNull;
	ObjectTemplate const * const ot = ObjectTemplateList::fetch(conversationAppearanceOverride);
	ServerObjectTemplate const * const sot = ot ? ot->asServerObjectTemplate() : nullptr;
	if(sot)
	{
		std::string const & sharedTemplateName = sot->getSharedTemplate();
		crc = Crc::calculate(sharedTemplateName.c_str());
	}

	return _npcStartConversation(env, self, player, npc, convoName, greeting, greetingOob, responses, crc);
}

//----------------------------------------------------------------------

jboolean JNICALL ScriptMethodsNpcNamespace::npcStartConversationAppearanceOverride(JNIEnv *env, jobject self, jlong player, jlong npc, jstring convoName, jobject greeting, jstring greetingOob, jobjectArray responses, jstring appearanceOverrideSharedTemplateName)
{
	JavaStringParam localAppearanceOverrideSharedTemplateName(appearanceOverrideSharedTemplateName);
	std::string appearanceOverrideSharedTemplateNameStr;
	if (!JavaLibrary::convert (localAppearanceOverrideSharedTemplateName, appearanceOverrideSharedTemplateNameStr))
		return JNI_FALSE;

	ObjectTemplate const * const ot = ObjectTemplateList::fetch(appearanceOverrideSharedTemplateNameStr);
	ServerObjectTemplate const * const sot = ot ? ot->asServerObjectTemplate() : nullptr;
	if(!sot)
	{
		return JNI_FALSE;
		WARNING (true, ("npcStartConversationAppearanceOverride: appearanceOverride string is not a server template name"));
	}
	std::string const & sharedTemplateName = sot->getSharedTemplate();
	uint32 const crc = Crc::calculate(sharedTemplateName.c_str());

	return _npcStartConversation(env, self, player, npc, convoName, greeting, greetingOob, responses, crc);
}

//----------------------------------------------------------------------

/**
 * Ends a player's current conversation.
 *
 * @param env		    Java environment
 * @param self		    class calling this function
 * @param player		player in conversation
 *
 * @return JNI_TRUE on success, JNI_FALSE on fail
*/
jboolean JNICALL ScriptMethodsNpcNamespace::npcEndConversation(JNIEnv *env, jobject self, jlong player)
{
	UNREF(self);

	TangibleObject * playerObject = nullptr;
	if (!JavaLibrary::getObject(player, playerObject))
		return JNI_FALSE;

	if (playerObject->isInNpcConversation())
	{
		playerObject->endNpcConversation();
	}

	return JNI_TRUE;
}	// JavaLibrary::npcEndConversation

//----------------------------------------------------------------------

/**
 * Has the NPC a player is in conversation say something to the player.
 *
 * @param env		    Java environment
 * @param self		    class calling this function
 * @param player		player in conversation
 * @param text			the message the npc will say to the player
 *
 * @return JNI_TRUE on success, JNI_FALSE on fail
*/
jboolean JNICALL ScriptMethodsNpcNamespace::npcSpeak(JNIEnv *env, jobject self, jlong player, jobject paramTextId, jstring paramOob)
{
	UNREF(self);

	TangibleObject * playerObject = nullptr;
	if (!JavaLibrary::getObject(player, playerObject))
		return JNI_FALSE;

	if (!playerObject->isInNpcConversation())
		return JNI_FALSE;

	StringId        textStringId;
	Unicode::String oob;

	if (paramTextId)
	{
		if (!ScriptConversion::convert(paramTextId, textStringId))
		{
			WARNING (true, ("ScriptMethodsNpc Unable to convert string id"));
			return JNI_FALSE;
		}
	}

	if (paramOob)
	{
		if (!JavaLibrary::convert (JavaStringParam (paramOob), oob))
		{
			WARNING (true, ("ScriptMethodsNpc unable to convert oob"));
			return JNI_FALSE;
		}
	}

	playerObject->sendNpcConversationMessage(textStringId, ProsePackage (), oob);
	return JNI_TRUE;
}	// JavaLibrary::npcSpeak

//----------------------------------------------------------------------

/**
	* Sets the available responses for a player to select in a conversation with
	* an NPC.
 *
 * @param env		    Java environment
 * @param self		    class calling this function
 * @param player		player in conversation
 * @param responses		responses available to the player
 *
 * @return JNI_TRUE on success, JNI_FALSE on fail
 */
jboolean JNICALL ScriptMethodsNpcNamespace::npcSetConversationResponses(JNIEnv *env, jobject self, jlong player, jobjectArray responses)
{
	UNREF(self);

	TangibleObject * playerObject = nullptr;
	if (!JavaLibrary::getObject(player, playerObject))
		return JNI_FALSE;

	if (!playerObject->isInNpcConversation())
		return JNI_FALSE;

	setNpcConversationResponses(env, playerObject, responses);

	return JNI_TRUE;
}	// JavaLibrary::npcSetConversationResponses

//----------------------------------------------------------------------

/**
 * Adds a response to the available responses for a player to select in a
 * conversation with an NPC.
 *
 * @param env		    Java environment
 * @param self		    class calling this function
 * @param player		player in conversation
 * @param response		response to add
 *
 * @return JNI_TRUE on success, JNI_FALSE on fail
 */
jboolean JNICALL ScriptMethodsNpcNamespace::npcAddConversationResponse(JNIEnv *env, jobject self, jlong player, jobject responseId, jstring responseOob)
{
	UNREF(self);

	TangibleObject * playerObject = nullptr;
	if (!JavaLibrary::getObject(player, playerObject))
		return JNI_FALSE;

	if (!playerObject->isInNpcConversation())
		return JNI_FALSE;

	StringId        stringId;
	ProsePackage    pp;

	if (responseId)
	{
		if (!ScriptConversion::convert(responseId, stringId))
			return JNI_FALSE;
	}

	if (responseOob)
	{
		Unicode::String oob;
		if (!JavaLibrary::convert (JavaStringParam (responseOob), oob))
			return JNI_FALSE;

		pp = getProseFromOob (oob);
		if (!pp.isValid ())
		{
			WARNING (true, ("ScriptMethodsNpc error getting prose from oob"));
			return JNI_FALSE;
		}
	}

	if (stringId.isInvalid () && !pp.isValid ())
	{
		WARNING (true, ("No stringid or pp"));
		return JNI_FALSE;
	}

	if (!playerObject->addNpcConversationResponse(stringId, pp))
		return JNI_FALSE;

	return JNI_TRUE;
}	// JavaLibrary::npcAddConversationResponse

//----------------------------------------------------------------------

/**
 * Removes a response from the available responses for a player to select in a
 * conversation with an NPC.
 *
 * @param env		    Java environment
 * @param self		    class calling this function
 * @param player		player in conversation
 * @param response		response to remove
 *
 * @return JNI_TRUE on success, JNI_FALSE on fail
 */
jboolean JNICALL ScriptMethodsNpcNamespace::npcRemoveConversationResponse(JNIEnv *env, jobject self, jlong player, jobject response)
{
	UNREF(self);

	TangibleObject * playerObject = nullptr;
	if (!JavaLibrary::getObject(player, playerObject))
		return JNI_FALSE;

	if (!playerObject->isInNpcConversation())
		return JNI_FALSE;

	StringId localResponse;
	if (!ScriptConversion::convert(response, localResponse))
		return JNI_FALSE;

	if (!playerObject->removeNpcConversationResponse(localResponse, ProsePackage ()))
		return JNI_FALSE;

	return JNI_TRUE;
}	// JavaLibrary::npcRemoveConversationResponse

//----------------------------------------------------------------------

/**
 * Determines if a creature is in an NPC conversation or not. The creature may
 * be a player or NPC.
 *
 * @param env		    Java environment
 * @param self		    class calling this function
 * @param creature		creature being asked about
 *
 * @return JNI_TRUE if creature is in an NPC conversation, JNI_FALSE if not
 */
jboolean JNICALL ScriptMethodsNpcNamespace::isInNpcConversation(JNIEnv *env, jobject self, jlong creature)
{
	UNREF(self);

	TangibleObject * tangibleObject = nullptr;
	if (!JavaLibrary::getObject(creature, tangibleObject))
		return JNI_FALSE;

	if (tangibleObject->isInNpcConversation())
		return JNI_TRUE;
	return JNI_FALSE;
}	// JavaLibrary::isInNpcConversation

//----------------------------------------------------------------------

/**
 * Returns the ids of any creatures a creature is in an NPC conversation with.
 *
 * @param env		    Java environment
 * @param self		    class calling this function
 * @param creature		creature being asked about
 *
 * @return an array of obj_ids the creature is in conversation with, or nullptr on error
*/
jlongArray JNICALL ScriptMethodsNpcNamespace::getNpcConversants(JNIEnv *env, jobject self, jlong creature)
{
	UNREF(self);

	TangibleObject * tangibleObject = nullptr;
	if (!JavaLibrary::getObject(creature, tangibleObject))
		return 0;

	const std::vector<NetworkId> & conversants = tangibleObject->getConversations();
	int count = conversants.size();

	// create an array of obj_ids and fill them in
	LocalLongArrayRefPtr objIds = createNewLongArray(count);
	if (objIds == LocalLongArrayRef::cms_nullPtr)
		return 0;

	jlong jlongTmp;
	for (int i = 0; i < count; ++i)
	{
		jlongTmp = (conversants[i]).getValue();
		setLongArrayRegion(*objIds, i, 1, &jlongTmp);
	}
	return objIds->getReturnValue();
}	// JavaLibrary::getNpcConversants

//----------------------------------------------------------------------

/**
 * Generates a random name for an object.
 *
 * @param env		    Java environment
 * @param self		    class calling this function
 * @param template    the name of the template to generate the name for
 * @return            The random name, or blank if one could not be generated
 */
jstring JNICALL ScriptMethodsNpcNamespace::generateRandomNameByTemplate(JNIEnv *env, jobject self, jstring i_templateName)
{
	UNREF(self);

	std::string templateName;
	if (!JavaLibrary::convert(JavaStringParam(i_templateName), templateName))
		return JavaString("").getReturnValue();

	Unicode::String randomName;

	const ObjectTemplate * const objectTemplate = ObjectTemplateList::fetch(templateName);
	if (objectTemplate)
	{
		const ServerCreatureObjectTemplate * const creatureTemplate = dynamic_cast<const ServerCreatureObjectTemplate *>(objectTemplate);
		if (creatureTemplate)
		{
			// generate a name
			randomName = NameManager::getInstance().generateRandomName(ConfigServerGame::getCharacterNameGeneratorDirectory(),creatureTemplate->getNameGeneratorType());
		}
		objectTemplate->releaseReference();
	}

	return JavaString(randomName).getReturnValue();
}	// JavaLibrary::speakString

// ----------------------------------------------------------------------

jstring JNICALL ScriptMethodsNpcNamespace::generateRandomNameByTable(JNIEnv *env, jobject self, jstring directory, jstring table)
{
	std::string myDirectory;
	if (!JavaLibrary::convert(JavaStringParam(directory), myDirectory))
		return JavaString("").getReturnValue();

	std::string myTable;
	if (!JavaLibrary::convert(JavaStringParam(table), myTable))
		return JavaString("").getReturnValue();

	Unicode::String randomName = NameManager::getInstance().generateRandomName(myDirectory, myTable);
	return JavaString(randomName).getReturnValue();
}

// ----------------------------------------------------------------------

jboolean JNICALL ScriptMethodsNpcNamespace::isNameReserved(JNIEnv *env, jobject self, jstring name)
{
	Unicode::String myName;
	if (!JavaLibrary::convert(JavaStringParam(name), myName))
		return false;

	return NameManager::getInstance().isNameReserved(myName);
}

// ----------------------------------------------------------------------

jboolean JNICALL ScriptMethodsNpcNamespace::isNameReservedIgnoreRules(JNIEnv *env, jobject self, jstring name, jobjectArray ignoreRules)
{
	Unicode::String myName;
	if (!JavaLibrary::convert(JavaStringParam(name), myName))
	{
		JAVA_THROW_SCRIPT_EXCEPTION(true, ("JavaLibrary::isNameReservedIgnoreRules passed invalid java string for the name"));
		return true;
	}

	std::vector<std::string> rulesToIgnore;
	if(!ScriptConversion::convert(ignoreRules, rulesToIgnore))
	{
		JAVA_THROW_SCRIPT_EXCEPTION(true, ("JavaLibrary::isNameReservedIgnoreRules passed invalid java string array"));
		return true;
	}

	std::string dummyReturn;
	return NameManager::getInstance().isNameReserved(myName, dummyReturn, rulesToIgnore);
}

// ----------------------------------------------------------------------

jboolean JNICALL ScriptMethodsNpcNamespace::npcEndConversationWithMessage(JNIEnv * /*env*/, jobject /*self*/, jlong player, jobject response, jstring responseOob)
{
	TangibleObject * playerObject = nullptr;
	if(!JavaLibrary::getObject(player, playerObject))
		return JNI_FALSE;
	
	if(!playerObject->isInNpcConversation())
		return JNI_FALSE;
	
	StringId responseStringId;
	bool responseIsValid = false;
	if(response)
	{
		responseIsValid = static_cast<bool>(ScriptConversion::convert(response, responseStringId));
	}
	
	bool oobIsValid = false;
	Unicode::String oob;
	if(responseOob)
	{
		oobIsValid = static_cast<bool>(JavaLibrary::convert(JavaStringParam(responseOob), oob));
	}
	
	if(responseIsValid || oobIsValid)
	{
		playerObject->endNpcConversation(responseStringId, oob);
	}
	else
	{
		playerObject->endNpcConversation();
	}

	return JNI_TRUE;
}

//----------------------------------------------------------------------

jboolean     JNICALL ScriptMethodsNpcNamespace::setNpcDifficulty(JNIEnv *env, jobject self, jlong npc, jlong difficulty)
{
	TangibleObject * npcObject = nullptr;
	if (!JavaLibrary::getObject(npc, npcObject))
		return JNI_FALSE;
	
	CreatureObject *creature = npcObject->asCreatureObject();

	if (!creature)
		return JNI_FALSE;
	
	if (difficulty < 0 || difficulty > 2)
		return JNI_FALSE;
	
	creature->setDifficulty(static_cast<CreatureObject::Difficulty> (difficulty));
	
	return JNI_TRUE;
}
