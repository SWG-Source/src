//========================================================================
//http://www.guidescope.com/home/bar.htm
// ScriptMethodsAnimation.cpp - implements script methods dealing with object
// movement.
//
// copyright 2001 Sony Online Entertainment
//
//========================================================================

#include "serverScript/FirstServerScript.h"
#include "serverScript/JavaLibrary.h"

#include "serverGame/CreatureObject.h"
#include "serverGame/CreatureController.h"
#include "serverGame/ServerObject.h"
#include "serverGame/TangibleObject.h"
#include "sharedNetworkMessages/MessageQueueString.h"
#include "sharedObject/Controller.h"
#include "sharedFoundation/GameControllerMessage.h"
#include "sharedObject/CellProperty.h"
#include "sharedObject/NetworkIdManager.h"

namespace ScriptMethodsAnimationNamespace
{
	bool install();

	void     JNICALL doAnimationAction (JNIEnv *env, jobject self, jlong target, jstring animationName);
	void     JNICALL setAnimationMood (JNIEnv *env, jobject self, jlong target, jstring moodName);
	jstring  JNICALL getAnimationMood (JNIEnv *env, jobject self, jlong target);
	jboolean JNICALL sitOnObject (JNIEnv *env, jobject self, jlong sitterId, jlong chairId, jint positionIndex);
	void     JNICALL setObjectAppearance (JNIEnv *env, jobject self, jlong target, jstring sharedTemplateName);
	void     JNICALL revertObjectAppearance (JNIEnv *env, jobject self, jlong target);
}

//========================================================================
// install
//========================================================================

bool ScriptMethodsAnimationNamespace::install()
{
const JNINativeMethod NATIVES[] = {
	#define JF(a,b,c) {a,b,(void*)(ScriptMethodsAnimationNamespace::c)}
	JF("_doAnimationAction", "(JLjava/lang/String;)V", doAnimationAction),
	JF("_setAnimationMood",  "(JLjava/lang/String;)V", setAnimationMood),
	JF("_getAnimationMood",  "(J)Ljava/lang/String;",  getAnimationMood),
	JF("_sitOnObject",       "(JJI)Z",   sitOnObject),
	JF("_setObjectAppearance",  "(JLjava/lang/String;)V", setObjectAppearance),
	JF("_revertObjectAppearance", "(J)V", revertObjectAppearance)
};

	return JavaLibrary::registerNatives(NATIVES, sizeof(NATIVES)/sizeof(NATIVES[0]));
}

//========================================================================
// class JavaLibrary JNI object location callback methods
//========================================================================

//----------------------------------------------------------------------


void JNICALL ScriptMethodsAnimationNamespace::doAnimationAction (JNIEnv *env, jobject self, jlong target, jstring animationName)
{
	UNREF(self);
	UNREF (env);

	TangibleObject * tangibleObject = 0;
	Controller * controller = 0;
	if (!JavaLibrary::getObjectController(target, tangibleObject, controller))
	{
		WARNING (true, ("doAnimationAction on bad TangibleObject/Controller id %d", NetworkId(target).getValue ()));
		return;
	}

	JavaStringParam localAnimationName(animationName);
	std::string animationNameStr;

	if (!JavaLibrary::convert(localAnimationName, animationNameStr))
	{
		WARNING (true, ("doAnimationAction unable to convert animationName to std::string"));
		return;
	}

	MessageQueueString * const msg = new MessageQueueString (animationNameStr);

	controller->appendMessage(CM_animationAction, 0, msg,
							  GameControllerMessageFlags::SEND |
							  GameControllerMessageFlags::RELIABLE |
							  GameControllerMessageFlags::DEST_ALL_CLIENT);
}

//----------------------------------------------------------------------
//@todo these are stubbed in right now.
void JNICALL ScriptMethodsAnimationNamespace::setAnimationMood (JNIEnv *env, jobject self, jlong target, jstring moodName)
{
	UNREF(env);
	UNREF(self);

	JavaStringParam localMoodName(moodName);

	ServerObject * targetObject = nullptr;
	if (!JavaLibrary::getObject(target, targetObject))
		return;

	std::string moodString;
	JavaLibrary::convert(localMoodName, moodString);

	CreatureObject * c = dynamic_cast<CreatureObject*>(targetObject);
	if (!c)
		return;
	c->setAnimationMood(moodString);
}

//----------------------------------------------------------------------

jstring JNICALL ScriptMethodsAnimationNamespace::getAnimationMood (JNIEnv *env, jobject self, jlong target)
{
	ServerObject * targetObject = nullptr;
	if (!JavaLibrary::getObject(target, targetObject))
		return 0;

	JavaString javaString("neutral");
	return javaString.getReturnValue();

	CreatureObject * c = dynamic_cast<CreatureObject*>(targetObject);
	if (!c)
		return 0;

	std::string moodString(c->getAnimationMood());
	JavaString javaReturn(moodString.c_str());
	return javaReturn.getReturnValue();
}

// ----------------------------------------------------------------------

jboolean JNICALL ScriptMethodsAnimationNamespace::sitOnObject (JNIEnv *env, jobject self, jlong sitterId, jlong chairId, jint positionIndex)
{
	UNREF(env);
	UNREF(self);
	UNREF(chairId);
	UNREF(positionIndex);

	//-- Get the creature who will be doing the sitting.
	CreatureObject *sitterObject = 0;
	if (!JavaLibrary::getObject (sitterId, sitterObject) || !sitterObject)
	{
		DEBUG_WARNING (true, ("sitOnObject(): Sitter object is nullptr."));
		return JNI_FALSE;
	}

	if (!sitterObject->isInWorld())
	{
		DEBUG_WARNING (true, ("sitOnObject(): Sitter object is not in the world."));
		return JNI_FALSE;
	}

	//-- Tell creature to sit on the closest chair to its current position.  This will be an issue on the
	//   client if no chair is within range (currently a 2 meter radius).
	NetworkId  creatureCellId;

	if (sitterObject->isInWorldCell())
		creatureCellId = NetworkId::cms_invalid;
	else
		creatureCellId = NON_NULL(sitterObject->getParentCell())->getOwner().getNetworkId();

	sitterObject->sitOnObject(creatureCellId, sitterObject->getPosition_w());

	//-- Return success.  At this point, the creature should at least go sit on the ground if not in the chair.
	return JNI_TRUE;
}

// ----------------------------------------------------------------------

void JNICALL ScriptMethodsAnimationNamespace::setObjectAppearance(JNIEnv *env, jobject self, jlong target, jstring sharedTemplateName)
{
	UNREF(env);
	UNREF(self);

	CreatureObject* creature = nullptr;

	if(!JavaLibrary::getObject(target, creature) || !creature)
	{
		DEBUG_WARNING(true, ("SetObjectAppearance() : Creature object was nullptr.\n"));
		return;
	}

	if(!creature->isInWorld())
	{
		DEBUG_WARNING(true, ("SetObjectAppearance(): Creature is not in the world.\n"));
		return;
	}
	std::string templateString;
	JavaStringParam sharedTemplateNameJString(sharedTemplateName);
	JavaLibrary::convert(sharedTemplateNameJString, templateString);
	
	CreatureController* controller = creature->getCreatureController();
	if(!controller)
	{
		DEBUG_WARNING(true, ("SetObjectAppearance(): Could not get creature controller.\n"));
		return;
	}

	controller->setAppearanceFromObjectTemplate(templateString);
}

// ----------------------------------------------------------------------

void JNICALL ScriptMethodsAnimationNamespace::revertObjectAppearance(JNIEnv *env, jobject self, jlong target)
{
	UNREF(env);
	UNREF(self);

	CreatureObject* creature = nullptr;

	if(!JavaLibrary::getObject(target, creature) || !creature)
	{
		DEBUG_WARNING(true, ("RevertObjectAppearance() : Creature object was nullptr.\n"));
		return;
	}

	if(!creature->isInWorld())
	{
		DEBUG_WARNING(true, ("RevertObjectAppearance(): Creature is not in the world.\n"));
		return;
	}

	CreatureController* controller = creature->getCreatureController();
	if(!controller)
	{
		DEBUG_WARNING(true, ("RevertObjectAppearance(): Could not get creature controller.\n"));
		return;
	}
	
	std::string const revert("none");
	controller->setAppearanceFromObjectTemplate(revert);
}

// ======================================================================
