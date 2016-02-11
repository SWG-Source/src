//========================================================================
//
// ScriptMethodsImageDesign.cpp - implements script methods dealing with imagedesigning.
//
// copyright 2001 Sony Online Entertainment
//
//========================================================================

#include "serverScript/FirstServerScript.h"
#include "serverScript/JavaLibrary.h"

#include "serverGame/Chat.h"
#include "serverGame/ServerImageDesignerManager.h"
#include "serverGame/ServerObject.h"
#include "sharedFoundation/Os.h"
#include "sharedGame/SharedImageDesignerManager.h"
#include "sharedGame/SharedStringIds.h"
#include "sharedNetworkMessages/ImageDesignStartMessage.h"
#include "sharedObject/Controller.h"


// ======================================================================
// ScriptMethodsImageDesignNamespace
// ======================================================================

namespace ScriptMethodsImageDesignNamespace
{
	bool install();

	void         JNICALL imagedesignStart(JNIEnv *env, jobject self, jlong jdesignerId, jlong jrecipientId, jlong jterminalId, jstring jCurrentHoloEmote);
	jboolean     JNICALL imagedesignValidated(JNIEnv *env, jobject self, jlong jdesignerId, jlong jrecipientId, jlong jterminalId, jint jstartingTime, jint jdesignType, jboolean jnewHairSet, jstring jnewHairAsset, jstring jhairCustomizationData, jint jdesignerRequiredCredits, jint jrecipientPaidCredits, jboolean jaccepted, jobjectArray jmorphChangesKeys, jfloatArray jmorphChangesValues, jobjectArray jindexChangesKeys, jintArray jindexChangesValues, jstring jholoEmote);
}


//========================================================================
// install
//========================================================================

bool ScriptMethodsImageDesignNamespace::install()
{
const JNINativeMethod NATIVES[] = {
	#define JF(a,b,c) {a,b,(void*)(ScriptMethodsImageDesignNamespace::c)}
	JF("_imagedesignStart", "(JJJLjava/lang/String;)V", imagedesignStart),
	JF("_imagedesignValidated", "(JJJIIZLjava/lang/String;Ljava/lang/String;IIZ[Ljava/lang/String;[F[Ljava/lang/String;[ILjava/lang/String;)Z", imagedesignValidated),
};

	return JavaLibrary::registerNatives(NATIVES, sizeof(NATIVES)/sizeof(NATIVES[0]));
}


//========================================================================
// class JavaLibrary JNI imagedesign callback methods
//========================================================================

void JNICALL ScriptMethodsImageDesignNamespace::imagedesignStart(JNIEnv *env, jobject self, jlong jdesignerId, jlong jrecipientId, jlong jterminalId, jstring jCurrentHoloEmote)
{
	UNREF(env);
	UNREF(self);
	ServerObject * designerObj = 0;
	if (!JavaLibrary::getObject(jdesignerId, designerObj))
	{
		WARNING(true, ("bad designer passed to JavaLibrary::imagedesignStart"));
		return;
	}

	ServerObject * recipientObj = 0;
	if (!JavaLibrary::getObject(jrecipientId, recipientObj))
	{
		WARNING(true, ("bad recipient passed to JavaLibrary::imagedesignStart"));
		return;
	}

	//the terminal can be nullptr (that means no ID terminal is being used, which is fine)
	ServerObject * terminalObj = 0;
	JavaLibrary::getObject(jterminalId, terminalObj);

	if(!designerObj || !recipientObj)
	{
		WARNING(true, ("could not resolve targets for JavaLibrary::imagedesignStart"));
		return;
	}

	SharedImageDesignerManager::Session newSession;
	newSession.designerId = designerObj->getNetworkId();
	newSession.recipientId = recipientObj->getNetworkId();
	newSession.startingTime = Os::getRealSystemTime();
	NetworkId const terminalId = terminalObj ? terminalObj->getNetworkId() : NetworkId::cms_invalid;
	newSession.terminalId = terminalId;
	SharedImageDesignerManager::startSession(newSession);

	std::string currentHoloEmote;
	if(jCurrentHoloEmote)
	{
		JavaStringParam localCurrentHoloEmote(jCurrentHoloEmote);
		IGNORE_RETURN(JavaLibrary::convert(localCurrentHoloEmote, currentHoloEmote));
	}

	Controller * const designerController = designerObj->getController();
	if(designerController)
	{
		ImageDesignStartMessage * const msg = new ImageDesignStartMessage(designerObj->getNetworkId(), recipientObj->getNetworkId(), terminalId, currentHoloEmote);
		designerController->appendMessage(CM_imageDesignerStart, 0, msg, GameControllerMessageFlags::SEND | GameControllerMessageFlags::RELIABLE | GameControllerMessageFlags::DEST_AUTH_CLIENT);
	}

	Controller * const recipientController = recipientObj->getController();
	if(recipientController && recipientController != designerController)
	{
		ImageDesignStartMessage * const msg = new ImageDesignStartMessage(designerObj->getNetworkId(), recipientObj->getNetworkId(), terminalId, currentHoloEmote);
		recipientController->appendMessage(CM_imageDesignerStart, 0, msg, GameControllerMessageFlags::SEND | GameControllerMessageFlags::RELIABLE | GameControllerMessageFlags::DEST_AUTH_CLIENT);
	}
}

//------------------------------------------------------------------------

jboolean JNICALL ScriptMethodsImageDesignNamespace::imagedesignValidated(JNIEnv *env, jobject self, jlong jdesignerId, jlong jrecipientId, jlong jterminalId, jint jstartingTime, jint jdesignType, jboolean jnewHairSet, jstring jnewHairAsset, jstring jhairCustomizationData, jint jdesignerRequiredCredits, jint jrecipientPaidCredits, jboolean jaccepted, jobjectArray jmorphChangesKeys, jfloatArray jmorphChangesValues, jobjectArray jindexChangesKeys, jintArray jindexChangesValues, jstring jholoEmote)
{
	UNREF(self);
	SharedImageDesignerManager::Session session;
	ServerObject * designerObj = 0;
	if (!JavaLibrary::getObject(jdesignerId, designerObj) || !designerObj)
	{
		WARNING(true, ("bad designer passed to JavaLibrary::imagedesignValidated"));
		return JNI_FALSE;
	}

	ServerObject * recipientObj = 0;
	if (!JavaLibrary::getObject(jrecipientId, recipientObj) || !recipientObj)
	{
		WARNING(true, ("bad recipient passed to JavaLibrary::imagedesignValidated"));
		return JNI_FALSE;
	}

	ServerObject * terminalObj = 0;
	JavaLibrary::getObject(jterminalId, terminalObj);

	session.designerId = designerObj->getNetworkId();
	session.recipientId = recipientObj->getNetworkId();
	session.terminalId = terminalObj ? terminalObj->getNetworkId() : NetworkId::cms_invalid;
	session.startingTime = jstartingTime;
	session.designType = static_cast<ImageDesignChangeMessage::DesignType>(jdesignType);
	session.newHairSet = jnewHairSet;
	std::string newHairAsset;
	if(jnewHairAsset)
	{
		JavaStringParam localNewHairAsset(jnewHairAsset);
		IGNORE_RETURN(JavaLibrary::convert(localNewHairAsset, newHairAsset));
	}
	session.newHairAsset = newHairAsset;
	std::string hairCustomizationData;
	if(jhairCustomizationData)
	{
		JavaStringParam localHairCustomizationData(jhairCustomizationData);
		IGNORE_RETURN(JavaLibrary::convert(localHairCustomizationData, hairCustomizationData));
	}
	session.hairCustomizationData = hairCustomizationData;
	session.designerRequiredCredits = jdesignerRequiredCredits;
	session.recipientPaidCredits = jrecipientPaidCredits;
	session.accepted = jaccepted;

	//if the session isn't valid, cancel it and don't apply changes
	if(!session.accepted)
	{
		Chat::sendSystemMessage(*designerObj, SharedStringIds::imagedesigner_session_not_valid, Unicode::emptyString);
		if(designerObj != recipientObj)
			Chat::sendSystemMessage(*recipientObj, SharedStringIds::imagedesigner_session_not_valid, Unicode::emptyString);
		ServerImageDesignerManager::cancelSession(session.designerId, session.recipientId);
		return JNI_TRUE;
	}

	//ensure we validate against the server's skill mod data
	SharedImageDesignerManager::SkillMods const & skillMods = ServerImageDesignerManager::getSkillModsForDesigner(session.designerId);
	session.bodySkillMod = skillMods.bodySkillMod;
	session.faceSkillMod = skillMods.faceSkillMod;
	session.markingsSkillMod = skillMods.markingsSkillMod;
	session.hairSkillMod = skillMods.hairSkillMod;

	std::string tempString;

	//get morph values
	std::map<std::string, float> morphChanges;
	int countKeys = env->GetArrayLength(jmorphChangesKeys);
	int countValues = env->GetArrayLength(jmorphChangesValues);
	if(countKeys != countValues)
	{
		WARNING(true, ("morph keys and values arrays are of diffent sizes"));
		return JNI_FALSE;
	}
	jfloat * const morphChangesValuesArray = env->GetFloatArrayElements(jmorphChangesValues, nullptr);
	if (morphChangesValuesArray == nullptr)
	{
		return JNI_FALSE;
	}
	std::vector<float> morphValues(morphChangesValuesArray, &morphChangesValuesArray[countValues]);
	env->ReleaseFloatArrayElements(jmorphChangesValues, morphChangesValuesArray, JNI_ABORT);
	for (int i = 0; i < countKeys; ++i)
	{
		JavaString jTempString(static_cast<jstring>(env->GetObjectArrayElement(jmorphChangesKeys, i)));
		if (JavaLibrary::convert(jTempString, tempString))
		{
			morphChanges[tempString] = morphValues[static_cast<unsigned int>(i)];
		}
	}
	session.morphChanges = morphChanges;

	//get index values
	std::map<std::string, int> indexChanges;
	countKeys = env->GetArrayLength(jindexChangesKeys);
	countValues = env->GetArrayLength(jindexChangesValues);
	if(countKeys != countValues)
	{
		WARNING(true, ("index keys and values arrays are of diffent sizes"));
		return JNI_FALSE;
	}
	jint * const indexChangesValuesArray = env->GetIntArrayElements(jindexChangesValues, nullptr);
	if (indexChangesValuesArray == nullptr)
	{
		return JNI_FALSE;
	}
	std::vector<int> indexValues(indexChangesValuesArray, &indexChangesValuesArray[countValues]);
	env->ReleaseIntArrayElements(jindexChangesValues, indexChangesValuesArray, JNI_ABORT);
	for (int j = 0; j < countKeys; ++j)
	{
		JavaString jTempString(static_cast<jstring>(env->GetObjectArrayElement(jindexChangesKeys, j)));
		if (JavaLibrary::convert(jTempString, tempString))
		{
			indexChanges[tempString] = indexValues[static_cast<unsigned int>(j)];
		}
	}
	session.indexChanges = indexChanges;

	//get holoemote values
	std::string holoEmote;
	if(jholoEmote)
	{
		JavaStringParam localHoloEmote(jholoEmote);
		IGNORE_RETURN(JavaLibrary::convert(localHoloEmote, holoEmote));
	}
	session.holoEmote = holoEmote;

	bool const result = ServerImageDesignerManager::makeChanges(session);
	if(!result)
	{
		Chat::sendSystemMessage(*designerObj, SharedStringIds::imagedesigner_session_not_valid, Unicode::emptyString);
		if(designerObj != recipientObj)
			Chat::sendSystemMessage(*recipientObj, SharedStringIds::imagedesigner_session_not_valid, Unicode::emptyString);
		ServerImageDesignerManager::cancelSession(session.designerId, session.recipientId);
	}
	if(result)
		return JNI_TRUE;
	else
		return JNI_FALSE;
}

//------------------------------------------------------------------------
