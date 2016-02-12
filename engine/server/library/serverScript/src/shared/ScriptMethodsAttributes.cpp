//========================================================================
//
// ScriptMethodsAttributes.cpp - implements script methods dealing with attributes.
//
// copyright 2001 Sony Online Entertainment
//
//========================================================================

#include "serverScript/FirstServerScript.h"
#include "serverScript/JavaLibrary.h"

#include "serverGame/AttribModNameManager.h"
#include "serverGame/ConfigServerGame.h"
#include "serverGame/CreatureController.h"
#include "serverGame/CreatureObject.h"
#include "serverGame/ServerWeaponObjectTemplate.h"
#include "serverGame/ServerWorld.h"
#include "sharedFoundation/Crc.h"
#include "sharedGame/GameObjectTypes.h"
#include "swgSharedUtility/CombatEngineData.h"

#include <climits>

using namespace JNIWrappersNamespace;


//========================================================================
// constants
//========================================================================

static const int ATTRIB_ERROR = INT_MIN;

//========================================================================
// namespace
//========================================================================

namespace ScriptMethodsAttributesNamespace
{
	bool install();
	jboolean addAttribModifierInternal(JNIEnv *env, CreatureObject * creature,jobject mod);

	jint         JNICALL getAttrib(JNIEnv *env, jobject self, jlong target, jint attrib);
	jint         JNICALL getUnmodifiedAttrib(JNIEnv *env, jobject self, jlong target, jint attrib);
	jint         JNICALL getMaxAttrib(JNIEnv *env, jobject self, jlong target, jint attrib);
	jint         JNICALL getWoundedMaxAttrib(JNIEnv *env, jobject self, jlong target, jint attrib);
	jint         JNICALL getUnmodifiedMaxAttrib(JNIEnv *env, jobject self, jlong target, jint attrib);
	jboolean     JNICALL setAttrib(JNIEnv *env, jobject self, jlong target, jint attrib, jint value);
	jboolean     JNICALL setMaxAttrib(JNIEnv *env, jobject self, jlong target, jint attrib, jint value);
	jboolean     JNICALL addToAttrib(JNIEnv *env, jobject self, jlong target, jint attrib, jint value);
	jboolean     JNICALL addToMaxAttrib(JNIEnv *env, jobject self, jlong target, jint attrib, jint value);
	jboolean     JNICALL addAttribModifier(JNIEnv *env, jobject self, jlong target, jobject mod);
	jboolean     JNICALL addAttribModifiers(JNIEnv *env, jobject self, jlong target, jobjectArray mods);
	jboolean     JNICALL hasAttribModifier(JNIEnv *env, jobject self, jlong target, jstring modName);
	jboolean     JNICALL hasSkillModModifier(JNIEnv *env, jobject self, jlong target, jstring modName);
	jobjectArray JNICALL getAttribModifiers(JNIEnv *env, jobject self, jlong target, jint attrib);
	jobjectArray JNICALL getAllAttribModifiers(JNIEnv *env, jobject self, jlong target);
	jboolean     JNICALL removeAttribModifier(JNIEnv *env, jobject self, jlong target, jstring modName);
	jboolean     JNICALL removeAllAttribModifiers(JNIEnv *env, jobject self, jlong target);
	jobjectArray JNICALL getAttribs(JNIEnv *env, jobject self, jlong target);
	jobjectArray JNICALL getUnmodifiedAttribs(JNIEnv *env, jobject self, jlong target);
	jobjectArray JNICALL getMaxAttribs(JNIEnv *env, jobject self, jlong target);
	jobjectArray JNICALL getWoundedMaxAttribs(JNIEnv *env, jobject self, jlong target);
	jobjectArray JNICALL getUnmodifiedMaxAttribs(JNIEnv *env, jobject self, jlong target);
	jboolean     JNICALL setAttribs(JNIEnv *env, jobject self, jlong target, jobjectArray values);
	jboolean     JNICALL setMaxAttribs(JNIEnv *env, jobject self, jlong target, jobjectArray values);
	jboolean     JNICALL addToAttribs(JNIEnv *env, jobject self, jlong target, jobjectArray values);
	jboolean     JNICALL addToMaxAttribs(JNIEnv *env, jobject self, jlong target, jobjectArray values);
	jint         JNICALL getHitpoints(JNIEnv *env, jobject self, jlong target);
	jint         JNICALL getMaxHitpoints(JNIEnv *env, jobject self, jlong target);
	jint         JNICALL getTotalHitpoints(JNIEnv *env, jobject self, jlong target);
	jboolean     JNICALL setHitpoints(JNIEnv *env, jobject self, jlong target, jint hitPoints);
	jboolean     JNICALL setMaxHitpoints(JNIEnv *env, jobject self, jlong target, jint hitPoints);
	jboolean     JNICALL setInvulnerableHitpoints(JNIEnv *env, jobject self, jlong target, jint hitPoints);
	jint         JNICALL getShockWound(JNIEnv *env, jobject self, jlong target);
	jboolean     JNICALL setShockWound(JNIEnv *env, jobject self, jlong target, jint wound);
	jboolean     JNICALL addShockWound(JNIEnv *env, jobject self, jlong target, jint wound);
	jboolean     JNICALL healShockWound(JNIEnv *env, jobject self, jlong target, jint value);
	jboolean     JNICALL drainAttributes(JNIEnv *env, jobject self, jlong target, jint action, jint mind);
	jint         JNICALL testDrainAttribute(JNIEnv *env, jobject self, jlong target, jint attribute, jint value);
	jboolean     JNICALL addBuffIcon(JNIEnv *env, jobject self, jlong target, jstring modName, jfloat time);
	jboolean     JNICALL clearBuffIcon(JNIEnv *env, jobject self, jlong target, jstring modName);
	jfloat       JNICALL getRegenRate(JNIEnv *env, jobject self, jlong target, jint attrib);
	void         JNICALL setRegenRate(JNIEnv *env, jobject self, jlong target, jint attrib, jfloat value);
}


//========================================================================
// install
//========================================================================

bool ScriptMethodsAttributesNamespace::install()
{
const JNINativeMethod NATIVES[] = {
	#define JF(a,b,c) {a,b,(void*)(ScriptMethodsAttributesNamespace::c)}
	JF("_getAttrib", "(JI)I", getAttrib),
	JF("_getUnmodifiedAttrib", "(JI)I", getUnmodifiedAttrib),
	JF("_getMaxAttrib", "(JI)I", getMaxAttrib),
	JF("_getWoundedMaxAttrib", "(JI)I", getWoundedMaxAttrib),
	JF("_getUnmodifiedMaxAttrib", "(JI)I", getUnmodifiedMaxAttrib),
	JF("_setAttrib", "(JII)Z", setAttrib),
	JF("_setMaxAttrib", "(JII)Z", setMaxAttrib),
	JF("_addToAttrib", "(JII)Z", addToAttrib),
	JF("_addToMaxAttrib", "(JII)Z", addToMaxAttrib),
	JF("_addAttribModifier", "(JLscript/attrib_mod;)Z", addAttribModifier),
	JF("_addAttribModifiers", "(J[Lscript/attrib_mod;)Z", addAttribModifiers),
	JF("_hasAttribModifier", "(JLjava/lang/String;)Z", hasAttribModifier),
	JF("_hasSkillModModifier", "(JLjava/lang/String;)Z", hasSkillModModifier),
	JF("_getAttribModifiers", "(JI)[Lscript/attrib_mod;", getAttribModifiers),
	JF("_getAllAttribModifiers", "(J)[Lscript/attrib_mod;", getAllAttribModifiers),
	JF("_removeAttribOrSkillModModifier", "(JLjava/lang/String;)Z", removeAttribModifier),
	JF("_removeAllAttribModifiers", "(J)Z", removeAllAttribModifiers),
	JF("_getAttribs", "(J)[Lscript/attribute;", getAttribs),
	JF("_getUnmodifiedAttribs", "(J)[Lscript/attribute;", getUnmodifiedAttribs),
	JF("_getMaxAttribs", "(J)[Lscript/attribute;", getMaxAttribs),
	JF("_getWoundedMaxAttribs", "(J)[Lscript/attribute;", getWoundedMaxAttribs),
	JF("_getUnmodifiedMaxAttribs", "(J)[Lscript/attribute;", getUnmodifiedMaxAttribs),
	JF("_setAttribs", "(J[Lscript/attribute;)Z", setAttribs),
	JF("_setMaxAttribs", "(J[Lscript/attribute;)Z", setMaxAttribs),
	JF("_addToAttribs", "(J[Lscript/attribute;)Z", addToAttribs),
	JF("_addToMaxAttribs", "(J[Lscript/attribute;)Z", addToMaxAttribs),
	JF("_getHitpoints", "(J)I", getHitpoints),
	JF("_getMaxHitpoints", "(J)I", getMaxHitpoints),
	JF("_getTotalHitpoints", "(J)I", getTotalHitpoints),
	JF("_setHitpoints", "(JI)Z", setHitpoints),
	JF("_setMaxHitpoints", "(JI)Z", setMaxHitpoints),
	JF("_setInvulnerableHitpoints", "(JI)Z", setInvulnerableHitpoints),
	JF("_getShockWound", "(J)I", getShockWound),
	JF("_setShockWound", "(JI)Z", setShockWound),
	JF("_addShockWound", "(JI)Z", addShockWound),
	JF("_healShockWound", "(JI)Z", healShockWound),
	JF("_drainAttributes", "(JII)Z", drainAttributes),
	JF("_testDrainAttribute", "(JII)I", testDrainAttribute),
	JF("_addBuffIcon", "(JLjava/lang/String;F)Z", addBuffIcon),
	JF("_clearBuffIcon", "(JLjava/lang/String;)Z", clearBuffIcon),
	JF("_getRegenRate", "(JI)F", getRegenRate),
	JF("_setRegenRate", "(JIF)V", setRegenRate),
};

	return JavaLibrary::registerNatives(NATIVES, sizeof(NATIVES)/sizeof(NATIVES[0]));
}


//========================================================================
// class JavaLibrary internal functions
//========================================================================

/**
 * Adds an attribute modifier to a creature.
 *
 * @param env		    Java environment
 * @param creature		creature to add the mod to
 * @param mod			the attrib_mod to add
 *
 * @return true on success, false on fail
 */
jboolean ScriptMethodsAttributesNamespace::addAttribModifierInternal(JNIEnv *env, CreatureObject * creature,
	jobject mod)
{
	if (creature == 0 || mod == 0)
		return JNI_FALSE;

	// get the mod's data
	AttribMod::AttribMod attribMod;
	if (!ScriptConversion::convert(mod, attribMod))
		return JNI_FALSE;

	if (AttribMod::isAttribMod(attribMod) || AttribMod::isDirectDamage(attribMod))
	{
		// verify data for mods that affect attribs
		if (attribMod.attrib < 0 || attribMod.attrib >= Attributes::NumberOfAttributes)
			return JNI_FALSE;

		if (!Attributes::isAttribPool(attribMod.attrib))
		{
			if (AttribMod::isDirectDamage(attribMod))
			{
				if (ConfigServerGame::getEnableAttribModWarnings())
				{
					WARNING(true, ("[script bug] addAttribModifier trying to permanently "
						"change non-pool attribute %d for creature %s. This is not allowed, "
						"since non-pool attribs don't regen.", attribMod.attrib,
						creature->getNetworkId().getValueString().c_str()));
				}
				JavaLibrary::throwInternalScriptError("JavaLibrary::addAttribModifier "
					"trying to permantly modify a non-pool attribute.");
			}
			if (attribMod.decay == AttribMod::AMDS_pool)
			{
				WARNING(true, ("[script bug] addAttribModifier trying to change a "
					"non-pool attribute %d for creature %s with decay = MOD_POOL",
					attribMod.attrib,
					creature->getNetworkId().getValueString().c_str()));
				return JNI_FALSE;
			}
			// non-pool mods must change both max and current
			attribMod.flags |=
				AttribMod::AMF_changeMax |
				AttribMod::AMF_attackCurrent |
				AttribMod::AMF_decayCurrent;
		}
	}
	else if (AttribMod::isSkillMod(attribMod))
	{
		// verify data for mods that affect skillmods
		if (attribMod.skill == 0)
		{
			WARNING(true, ("[script bug] addAttribModifier adding skillmod mod "
				"with no skillmod name"));
			return JNI_FALSE;
		}
	}

	if ((attribMod.flags & AttribMod::AMF_triggerOnDone) && attribMod.tag == 0)
	{
		DEBUG_WARNING(true, ("[script bug] addAttribModifier adding mod with "
			"end-trigger but has no name! The trigger will not be called "
			"when the mod ends."));
	}

	creature->addAttributeModifier(attribMod);
	return JNI_TRUE;
}	// JavaLibrary::addAttribModifier


//========================================================================
// class JavaLibrary JNI attribute callback methods
//========================================================================

/**
 * Returns a creature's attribute.
 *
 * @param env		    Java environment
 * @param self		    class calling this function
 * @param target	    id of creature to access
 * @param attrib		attribute we are interested in
 *
 * @return the attribute value, or ATTRIB_ERROR on error
 */
jint JNICALL ScriptMethodsAttributesNamespace::getAttrib(JNIEnv *env, jobject self, jlong target, jint attrib)
{
	UNREF(self);

	if (attrib < 0 || attrib >= Attributes::NumberOfAttributes)
		return ATTRIB_ERROR;

	CreatureObject * creature = 0;
	if (!JavaLibrary::getObject(target, creature))
		return ATTRIB_ERROR;

	return creature->getAttribute(static_cast<Attributes::Enumerator>(attrib));
}	// JavaLibrary::getAttrib

/**
 * Returns a creature's unmodified attribute.
 *
 * @param env		    Java environment
 * @param self		    class calling this function
 * @param target	    id of creature to access
 * @param attrib		attribute we are interested in
 *
 * @return the attribute value, or ATTRIB_ERROR on error
 */
jint JNICALL ScriptMethodsAttributesNamespace::getUnmodifiedAttrib(JNIEnv *env, jobject self, jlong target, jint attrib)
{
	UNREF(self);

	if (attrib < 0 || attrib >= Attributes::NumberOfAttributes)
		return ATTRIB_ERROR;

	CreatureObject * creature = 0;
	if (!JavaLibrary::getObject(target, creature))
		return ATTRIB_ERROR;

	return creature->getUnmodifiedAttribute(static_cast<Attributes::Enumerator>(attrib));
}	// JavaLibrary::getUnmodifiedAttrib

/**
 * Returns a creature's max attribute.
 *
 * @param env		    Java environment
 * @param self		    class calling this function
 * @param target	    id of creature to access
 * @param attrib		attribute we are interested in
 *
 * @return the max attribute value, or ATTRIB_ERROR on error
 */
jint JNICALL ScriptMethodsAttributesNamespace::getMaxAttrib(JNIEnv *env, jobject self, jlong target, jint attrib)
{
	UNREF(self);

	if (attrib < 0 || attrib >= Attributes::NumberOfAttributes)
		return ATTRIB_ERROR;

	CreatureObject * creature = 0;
	if (!JavaLibrary::getObject(target, creature))
		return ATTRIB_ERROR;

	return creature->getMaxAttribute(static_cast<Attributes::Enumerator>(attrib));
}	// JavaLibrary::getMaxAttrib

/**
 * Returns a creature's max attribute, modified by wounds.
 *
 * @param env		    Java environment
 * @param self		    class calling this function
 * @param target	    id of creature to access
 * @param attrib		attribute we are interested in
 *
 * @return the max attribute value, or ATTRIB_ERROR on error
 */
jint JNICALL ScriptMethodsAttributesNamespace::getWoundedMaxAttrib(JNIEnv *env, jobject self, jlong target, jint attrib)
{
	UNREF(self);

	if (attrib < 0 || attrib >= Attributes::NumberOfAttributes)
		return ATTRIB_ERROR;

	CreatureObject * creature = 0;
	if (!JavaLibrary::getObject(target, creature))
		return ATTRIB_ERROR;

	return creature->getMaxAttribute(static_cast<Attributes::Enumerator>(attrib));
}	// JavaLibrary::getWoundedMaxAttrib

/**
 * Returns a creature's unmodified max attribute.
 *
 * @param env		    Java environment
 * @param self		    class calling this function
 * @param target	    id of creature to access
 * @param attrib		attribute we are interested in
 *
 * @return the max attribute value, or ATTRIB_ERROR on error
 */
jint JNICALL ScriptMethodsAttributesNamespace::getUnmodifiedMaxAttrib(JNIEnv *env, jobject self, jlong target, jint attrib)
{
	UNREF(self);

	if (attrib < 0 || attrib >= Attributes::NumberOfAttributes)
		return ATTRIB_ERROR;

	CreatureObject * creature = 0;
	if (!JavaLibrary::getObject(target, creature))
		return ATTRIB_ERROR;

	return creature->getUnmodifiedMaxAttribute(static_cast<Attributes::Enumerator>(attrib));
}	// JavaLibrary::getUnmodifiedMaxAttrib

/**
 * Sets a creatue's attribute.
 *
 * @param env		    Java environment
 * @param self		    class calling this function
 * @param target	    id of creature to access
 * @param attrib		attribute we are interested in
 * @param value			value to set the attribute to
 *
 * @return true on success, false on fail
 */
jboolean JNICALL ScriptMethodsAttributesNamespace::setAttrib(JNIEnv *env, jobject self, jlong target, jint attrib, jint value)
{
	UNREF(self);

	if (attrib < 0 || attrib >= Attributes::NumberOfAttributes)
		return JNI_FALSE;

	CreatureObject * creature = 0;
	if (!JavaLibrary::getObject(target, creature))
		return JNI_FALSE;

	if (!Attributes::isAttribPool(attrib) && creature->isPlayerControlled())
	{
		WARNING(true, ("[designer bug] setAttrib called on player %s for "
			"non-pool attribute %d", creature->getNetworkId().getValueString().c_str(),
			attrib));
		return JNI_FALSE;
	}

	creature->setAttribute(static_cast<Attributes::Enumerator>(attrib),
		static_cast<Attributes::Value>(value));
	return JNI_TRUE;
}	// JavaLibrary::setAttrib

/**
 * Sets a creatue's max attribute.
 *
 * @param env		    Java environment
 * @param self		    class calling this function
 * @param target	    id of creature to access
 * @param attrib		attribute we are interested in
 * @param value			value to set the max attribute to
 *
 * @return true on success, false on fail
 */
jboolean JNICALL ScriptMethodsAttributesNamespace::setMaxAttrib(JNIEnv *env, jobject self, jlong target, jint attrib, jint value)
{
	UNREF(self);

	if (attrib < 0 || attrib >= Attributes::NumberOfAttributes)
		return JNI_FALSE;

	CreatureObject * creature = 0;
	if (!JavaLibrary::getObject(target, creature))
		return JNI_FALSE;

	creature->setMaxAttribute(static_cast<Attributes::Enumerator>(attrib),
		static_cast<Attributes::Value>(value));
	return JNI_TRUE;
}	// JavaLibrary::setMaxAttrib

/**
 * Adds a value to a creature's attribute.
 *
 * @param env		    Java environment
 * @param self		    class calling this function
 * @param target	    id of creature to access
 * @param attrib		attribute we are interested in
 * @param value			value to add to the attribute
 *
 * @return true on success, false on fail
 */
jboolean JNICALL ScriptMethodsAttributesNamespace::addToAttrib(JNIEnv *env, jobject self, jlong target, jint attrib, jint value)
{
	UNREF(self);

	if (attrib < 0 || attrib >= Attributes::NumberOfAttributes)
		return JNI_FALSE;

	CreatureObject * creature = 0;
	if (!JavaLibrary::getObject(target, creature))
		return JNI_FALSE;

	if (!Attributes::isAttribPool(attrib) && creature->isPlayerControlled())
	{
		WARNING(true, ("[designer bug] addToAttrib called on player %s for "
			"non-pool attribute %d", creature->getNetworkId().getValueString().c_str(),
			attrib));
		return JNI_FALSE;
	}

	creature->setAttribute(static_cast<Attributes::Enumerator>(attrib),
		static_cast<Attributes::Value>((creature->getAttribute(
		static_cast<Attributes::Enumerator>(attrib)) + value)));
	return JNI_TRUE;
}	// JavaLibrary::addToAttrib

/**
 * Adds a value to a creature's max attribute.
 *
 * @param env		    Java environment
 * @param self		    class calling this function
 * @param target	    id of creature to access
 * @param attrib		attribute we are interested in
 * @param value			value to add to the max attribute
 *
 * @return true on success, false on fail
 */
jboolean JNICALL ScriptMethodsAttributesNamespace::addToMaxAttrib(JNIEnv *env, jobject self, jlong target, jint attrib, jint value)
{
	UNREF(self);

	if (attrib < 0 || attrib >= Attributes::NumberOfAttributes)
		return JNI_FALSE;

	CreatureObject * creature = 0;
	if (!JavaLibrary::getObject(target, creature))
		return JNI_FALSE;

	creature->setMaxAttribute(static_cast<Attributes::Enumerator>(attrib),
		static_cast<Attributes::Value>((creature->getUnmodifiedMaxAttribute(
		static_cast<Attributes::Enumerator>(attrib)) + value)));
	return JNI_TRUE;
}	// JavaLibrary::addToMaxAttrib

/**
 * Adds an attribute modifier to a creature.
 *
 * @param env		    Java environment
 * @param self		    class calling this function
 * @param target	    id of creature to access
 * @param mod          the attrib_mod to add
 *
 * @return true on success, false on fail
 */
jboolean JNICALL ScriptMethodsAttributesNamespace::addAttribModifier(JNIEnv *env, jobject self, jlong target, jobject mod)
{
	UNREF(self);

	if (target == 0 || mod == 0)
		return JNI_FALSE;

	CreatureObject * creature = 0;
	if (!JavaLibrary::getObject(target, creature))
		return JNI_FALSE;

	return addAttribModifierInternal(env, creature, mod);
}	// JavaLibrary::addAttribModifier

/**
 * Adds a series of attribute modifiers to a creature.
 *
 * @param env		    Java environment
 * @param self		    class calling this function
 * @param target	    id of creature to access
 * @param mods			the attrib_mods we want to apply
 *
 * @return true on success, false on fail
 */
jboolean JNICALL ScriptMethodsAttributesNamespace::addAttribModifiers(JNIEnv *env, jobject self, jlong target, jobjectArray mods)
{
	UNREF(self);

	if (target == 0 || mods == 0)
		return JNI_FALSE;

	CreatureObject * creature = 0;
	if (!JavaLibrary::getObject(target, creature))
		return JNI_FALSE;

	jint count = env->GetArrayLength(mods);
	for (int i = 0; i < count; ++i)
	{
		LocalRef mod(env->GetObjectArrayElement(mods, i));
		if (addAttribModifierInternal(env, creature, mod.getValue()) == JNI_FALSE)
			return JNI_FALSE;
	}
	return JNI_TRUE;
}	// JavaLibrary::addAttribModifiers

/**
 * Tests to see if a creature has a given attrib mod or mod group.
 *
 * @param env		    Java environment
 * @param self		    class calling this function
 * @param target	    id of creature to test
 * @param modName		name of the mod we want
 *
 * @return true if the creature has the mod, false if not
 */
jboolean JNICALL ScriptMethodsAttributesNamespace::hasAttribModifier(JNIEnv *env, jobject self, jlong target, jstring modName)
{
	UNREF(self);

	if (target == 0 || modName == 0)
		return JNI_FALSE;

	JavaStringParam jmodName(modName);
	std::string name;
	if (!JavaLibrary::convert(jmodName, name))
		return JNI_FALSE;

	const CreatureObject * creature = nullptr;
	if (!JavaLibrary::getObject(target, creature))
		return JNI_FALSE;

	const AttribMod::AttribMod * mod = creature->getAttributeModifier(name);
	if (mod != nullptr && AttribMod::isAttribMod(*mod))
		return JNI_TRUE;

	return JNI_FALSE;
}	// JavaLibrary::hasAttribModifier

/**
 * Tests to see if a creature has a given skill mod or mod group.
 *
 * @param env		    Java environment
 * @param self		    class calling this function
 * @param target	    id of creature to test
 * @param modName		name of the mod we want
 *
 * @return true if the creature has the mod, false if not
 */
jboolean JNICALL ScriptMethodsAttributesNamespace::hasSkillModModifier(JNIEnv *env, jobject self, jlong target, jstring modName)
{
	UNREF(self);

	if (target == 0 || modName == 0)
		return JNI_FALSE;

	JavaStringParam jmodName(modName);
	std::string name;
	if (!JavaLibrary::convert(jmodName, name))
		return JNI_FALSE;

	const CreatureObject * creature = nullptr;
	if (!JavaLibrary::getObject(target, creature))
		return JNI_FALSE;

	const AttribMod::AttribMod * mod = creature->getAttributeModifier(name);
	if (mod != nullptr && AttribMod::isSkillMod(*mod))
		return JNI_TRUE;

	return JNI_FALSE;
}	// JavaLibrary::hasSkillModModifier

/**
 * Returns the attribute modifiers on a creature for a given attribute.
 *
 * @param env		    Java environment
 * @param self		    class calling this function
 * @param target	    id of creature to access
 * @param attrib		attribute we are interested in
 *
 * @return the attribute modifiers for the creature, or nullptr if it has none
 */
jobjectArray JNICALL ScriptMethodsAttributesNamespace::getAttribModifiers(JNIEnv *env, jobject self, jlong target, jint attrib)
{
	typedef std::map<uint32, CreatureMod> ModList;

	UNREF(self);

	if (attrib < 0 || attrib >= Attributes::NumberOfAttributes)
		return 0;

	const CreatureObject * creature = 0;
	if (!JavaLibrary::getObject(target, creature))
		return 0;

	std::vector<AttribMod::AttribMod> destMods;
	const ModList & sourceMods = creature->getAttributeModifiers();
	for (ModList::const_iterator iter = sourceMods.begin(); iter != sourceMods.end();
		++iter)
	{
		if (AttribMod::isAttribMod((*iter).second.mod) &&
			(*iter).second.mod.attrib == attrib)
		{
			destMods.push_back((*iter).second.mod);
		}
	}

	LocalObjectArrayRefPtr result;
	if (!ScriptConversion::convert(destMods, result))
		return 0;

	return result->getReturnValue();
}	// JavaLibrary::getAttribModifiers

/**
 * Returns the attribute modifiers on a creature.
 *
 * @param env		    Java environment
 * @param self		    class calling this function
 * @param target	    id of creature to access
 *
 * @return the attribute modifiers for the creature, or nullptr if it has none
 */
jobjectArray JNICALL ScriptMethodsAttributesNamespace::getAllAttribModifiers(JNIEnv *env, jobject self, jlong target)
{
	typedef std::map<uint32, CreatureMod> ModList;

	UNREF(self);

	const CreatureObject * creature = 0;
	if (!JavaLibrary::getObject(target, creature))
		return 0;

	std::vector<AttribMod::AttribMod> destMods;
	const ModList & sourceMods = creature->getAttributeModifiers();
	for (ModList::const_iterator iter = sourceMods.begin(); iter != sourceMods.end();
		++iter)
	{
		if (AttribMod::isAttribMod((*iter).second.mod))
			destMods.push_back((*iter).second.mod);
	}

	LocalObjectArrayRefPtr result;
	if (!ScriptConversion::convert(destMods, result))
		return 0;

	return result->getReturnValue();
}	// JavaLibrary::getAllAttribModifiers

/**
 * Removes the attrib/skill mod(s) with a given name.
 *
 * @param env		    Java environment
 * @param self		    class calling this function
 * @param target	    id of creature
 * @param modName		name of the mod we want to remove
 *
 * @return true on success, false on fail
 */
jboolean JNICALL ScriptMethodsAttributesNamespace::removeAttribModifier(JNIEnv *env, jobject self, jlong target, jstring modName)
{
	UNREF(self);

	if (target == 0 || modName == 0)
		return JNI_FALSE;

	JavaStringParam jmodName(modName);
	std::string name;
	if (!JavaLibrary::convert(jmodName, name))
		return JNI_FALSE;

	CreatureObject * creature = nullptr;
	if (!JavaLibrary::getObject(target, creature))
		return JNI_FALSE;

	creature->removeAttributeModifier(name);
	return JNI_TRUE;
}	// JavaLibrary::removeAttribModifier

/**
 * Removes all the attribute modifiers from a creature.
 *
 * @param env		    Java environment
 * @param self		    class calling this function
 * @param target	    id of creature to access
 *
 * @return JNI_TRUE on success, JNI_FALSE on fail
 */
jboolean JNICALL ScriptMethodsAttributesNamespace::removeAllAttribModifiers(JNIEnv *env, jobject self, jlong target)
{
	UNREF(self);

	CreatureObject * creature = 0;
	if (!JavaLibrary::getObject(target, creature))
		return JNI_FALSE;

	creature->removeAllAttributeModifiers();

	return JNI_TRUE;
}	// JavaLibrary::removeAllAttribModifiers

/**
 * Returns all the attributes for a creature.
 *
 * @param env		    Java environment
 * @param self		    class calling this function
 * @param target	    id of creature to access
 *
 * @return the attributes for the creature
 */
jobjectArray JNICALL ScriptMethodsAttributesNamespace::getAttribs(JNIEnv *env, jobject self, jlong target)
{
	UNREF(self);

	CreatureObject * creature = 0;
	if (!JavaLibrary::getObject(target, creature))
		return 0;

	// create the array of attributes
	LocalObjectArrayRefPtr attribs = createNewObjectArray(Attributes::NumberOfAttributes, JavaLibrary::getClsAttribute());
	if (env->ExceptionCheck())
	{
		env->ExceptionDescribe();
		return 0;
	}

	// set the array elements
	for (int i = 0; i < Attributes::NumberOfAttributes; ++i)
	{
		LocalRefPtr attrib = createNewObject(JavaLibrary::getClsAttribute(), JavaLibrary::getMidAttribute(),
			i, creature->getAttribute(static_cast<Attributes::Enumerator>(i)));
		if (env->ExceptionCheck())
		{
			env->ExceptionDescribe();
			return 0;
		}
		setObjectArrayElement(*attribs, i, *attrib);
	}
	return attribs->getReturnValue();
}	// JavaLibrary::getAttribs

/**
 * Returns all the unmodified attributes for a creature.
 *
 * @param env		    Java environment
 * @param self		    class calling this function
 * @param target	    id of creature to access
 *
 * @return the attributes for the creature
 */
jobjectArray JNICALL ScriptMethodsAttributesNamespace::getUnmodifiedAttribs(JNIEnv *env, jobject self, jlong target)
{
	UNREF(self);

	CreatureObject * creature = 0;
	if (!JavaLibrary::getObject(target, creature))
		return 0;

	// create the array of attributes
	LocalObjectArrayRefPtr attribs = createNewObjectArray(
		Attributes::NumberOfAttributes, JavaLibrary::getClsAttribute());
	if (env->ExceptionCheck())
	{
		env->ExceptionDescribe();
		return 0;
	}

	// set the array elements
	for (int i = 0; i < Attributes::NumberOfAttributes; ++i)
	{
		LocalRefPtr attrib = createNewObject(JavaLibrary::getClsAttribute(), JavaLibrary::getMidAttribute(),
			i, creature->getUnmodifiedAttribute(static_cast<Attributes::Enumerator>(i)));
		if (env->ExceptionCheck())
		{
			env->ExceptionDescribe();
			return 0;
		}
		setObjectArrayElement(*attribs, i, *attrib);
	}
	return attribs->getReturnValue();
}	// JavaLibrary::getUnmodifiedAttribs

/**
 * Returns all the max attributes for a creature.
 *
 * @param env		    Java environment
 * @param self		    class calling this function
 * @param target	    id of creature to access
 *
 * @return the max attributes for the creature
 */
jobjectArray JNICALL ScriptMethodsAttributesNamespace::getMaxAttribs(JNIEnv *env, jobject self, jlong target)
{
	UNREF(self);

	CreatureObject * creature = 0;
	if (!JavaLibrary::getObject(target, creature))
		return 0;

	// create the array of attributes
	LocalObjectArrayRefPtr attribs = createNewObjectArray(
		Attributes::NumberOfAttributes, JavaLibrary::getClsAttribute());
	if (env->ExceptionCheck())
	{
		env->ExceptionDescribe();
		return 0;
	}

	// set the array elements
	for (int i = 0; i < Attributes::NumberOfAttributes; ++i)
	{
		LocalRefPtr attrib = createNewObject(JavaLibrary::getClsAttribute(), JavaLibrary::getMidAttribute(),
			i, creature->getMaxAttribute(static_cast<Attributes::Enumerator>(i)));
		if (env->ExceptionCheck())
		{
			env->ExceptionDescribe();
			return 0;
		}
		setObjectArrayElement(*attribs, i, *attrib);
	}
	return attribs->getReturnValue();
}	// JavaLibrary::getMaxAttribs

/**
 * Returns all the max attributes for a creature, modified by wounds.
 *
 * @param env		    Java environment
 * @param self		    class calling this function
 * @param target	    id of creature to access
 *
 * @return the max attributes for the creature
 */
jobjectArray JNICALL ScriptMethodsAttributesNamespace::getWoundedMaxAttribs(JNIEnv *env, jobject self, jlong target)
{
	UNREF(self);

	CreatureObject * creature = 0;
	if (!JavaLibrary::getObject(target, creature))
		return 0;

	// create the array of attributes
	LocalObjectArrayRefPtr attribs = createNewObjectArray(
		Attributes::NumberOfAttributes, JavaLibrary::getClsAttribute());
	if (env->ExceptionCheck())
	{
		env->ExceptionDescribe();
		return 0;
	}

	// set the array elements
	for (int i = 0; i < Attributes::NumberOfAttributes; ++i)
	{
		LocalRefPtr attrib = createNewObject(JavaLibrary::getClsAttribute(), JavaLibrary::getMidAttribute(),
			i, creature->getMaxAttribute(static_cast<Attributes::Enumerator>(i)));
		if (env->ExceptionCheck())
		{
			env->ExceptionDescribe();
			return 0;
		}
		setObjectArrayElement(*attribs, i, *attrib);
	}
	return attribs->getReturnValue();
}	// JavaLibrary::getWoundedMaxAttribs

/**
 * Returns all the unmodified max attributes for a creature.
 *
 * @param env		    Java environment
 * @param self		    class calling this function
 * @param target	    id of creature to access
 *
 * @return the max attributes for the creature
 */
jobjectArray JNICALL ScriptMethodsAttributesNamespace::getUnmodifiedMaxAttribs(JNIEnv *env, jobject self, jlong target)
{
	UNREF(self);

	CreatureObject * creature = 0;
	if (!JavaLibrary::getObject(target, creature))
		return 0;

	// create the array of attributes
	LocalObjectArrayRefPtr attribs = createNewObjectArray(
		Attributes::NumberOfAttributes, JavaLibrary::getClsAttribute());
	if (env->ExceptionCheck())
	{
		env->ExceptionDescribe();
		return 0;
	}

	// set the array elements
	for (int i = 0; i < Attributes::NumberOfAttributes; ++i)
	{
		LocalRefPtr attrib = createNewObject(JavaLibrary::getClsAttribute(), JavaLibrary::getMidAttribute(),
			i, creature->getUnmodifiedMaxAttribute(static_cast<Attributes::Enumerator>(
			i)));
		if (env->ExceptionCheck())
		{
			env->ExceptionDescribe();
			return 0;
		}
		setObjectArrayElement(*attribs, i, *attrib);
	}
	return attribs->getReturnValue();
}	// JavaLibrary::getUnmodifiedMaxAttribs

/**
 * Sets the attributes for a creature.
 *
 * @param env		    Java environment
 * @param self		    class calling this function
 * @param target	    id of creature to access
 * @param values		attribute/value pairs to set
 *
 * @return true on success, false on fail
 */
jboolean JNICALL ScriptMethodsAttributesNamespace::setAttribs(JNIEnv *env, jobject self, jlong target, jobjectArray values)
{
	UNREF(self);

	CreatureObject * creature = 0;
	if (!JavaLibrary::getObject(target, creature))
		return JNI_FALSE;

	int count = env->GetArrayLength(values);
	for (int i = 0; i < count; ++i)
	{
		LocalRefPtr attrib = getObjectArrayElement(LocalObjectArrayRefParam(values), i);
		if (attrib == LocalRef::cms_nullPtr)
			return JNI_FALSE;
		int type = getIntField(*attrib, JavaLibrary::getFidAttributeType());
		if (type < 0 || type >= Attributes::NumberOfAttributes)
			return JNI_FALSE;

		creature->setAttribute(static_cast<Attributes::Enumerator>(type),
			static_cast<Attributes::Value>(getIntField(*attrib, JavaLibrary::getFidAttributeValue())));
	}
	return JNI_TRUE;
}	// JavaLibrary::setAttribs

/**
 * Sets the max attributes for a creature.
 *
 * @param env		    Java environment
 * @param self		    class calling this function
 * @param target	    id of creature to access
 * @param values		attribute/value pairs to set
 *
 * @return true on success, false on fail
 */
jboolean JNICALL ScriptMethodsAttributesNamespace::setMaxAttribs(JNIEnv *env, jobject self, jlong target, jobjectArray values)
{
	UNREF(self);

	CreatureObject * creature = 0;
	if (!JavaLibrary::getObject(target, creature))
		return JNI_FALSE;

	int count = env->GetArrayLength(values);
	for (int i = 0; i < count; ++i)
	{
		LocalRefPtr attrib = getObjectArrayElement(LocalObjectArrayRefParam(values), i);
		if (attrib == LocalRef::cms_nullPtr)
			return JNI_FALSE;
		int type = getIntField(*attrib, JavaLibrary::getFidAttributeType());
		if (type < 0 || type >= Attributes::NumberOfAttributes)
			return JNI_FALSE;
		creature->setMaxAttribute(static_cast<Attributes::Enumerator>(type),
			static_cast<Attributes::Value>(getIntField(*attrib,
			JavaLibrary::getFidAttributeValue())), false);
	}
	return JNI_TRUE;
}	// JavaLibrary::setMaxAttribs

/**
 * Adds values to the attributes for a creature.
 *
 * @param env		    Java environment
 * @param self		    class calling this function
 * @param target	    id of creature to access
 * @param values		attribute/value pairs to add
 *
 * @return true on success, false on fail
 */
jboolean JNICALL ScriptMethodsAttributesNamespace::addToAttribs(JNIEnv *env, jobject self, jlong target, jobjectArray values)
{
	UNREF(self);

	CreatureObject * creature = 0;
	if (!JavaLibrary::getObject(target, creature))
		return JNI_FALSE;

	int count = env->GetArrayLength(values);
	for (int i = 0; i < count; ++i)
	{
		LocalRefPtr attrib = getObjectArrayElement(LocalObjectArrayRefParam(values), i);
		if (attrib == LocalRef::cms_nullPtr)
			return JNI_FALSE;
		int type = getIntField(*attrib, JavaLibrary::getFidAttributeType());
		if (type < 0 || type >= Attributes::NumberOfAttributes)
			return JNI_FALSE;

		int value = creature->getAttribute(static_cast<Attributes::Enumerator>(type));
		value += getIntField(*attrib, JavaLibrary::getFidAttributeValue());
		creature->setAttribute(static_cast<Attributes::Enumerator>(type),
			static_cast<Attributes::Value>(value));
	}
	return JNI_TRUE;
}	// JavaLibrary::addToAttribs

/**
 * Adds values to the max attributes for a creature.
 *
 * @param env		    Java environment
 * @param self		    class calling this function
 * @param target	    id of creature to access
 * @param values		attribute/value pairs to add
 *
 * @return true on success, false on fail
 */
jboolean JNICALL ScriptMethodsAttributesNamespace::addToMaxAttribs(JNIEnv *env, jobject self, jlong target, jobjectArray values)
{
	UNREF(self);

	CreatureObject * creature = 0;
	if (!JavaLibrary::getObject(target, creature))
		return JNI_FALSE;

	int count = env->GetArrayLength(values);
	for (int i = 0; i < count; ++i)
	{
		LocalRefPtr attrib = getObjectArrayElement(LocalObjectArrayRefParam(values), i);
		if (attrib == LocalRef::cms_nullPtr)
			return JNI_FALSE;
		int type = getIntField(*attrib, JavaLibrary::getFidAttributeType());
		if (type < 0 || type >= Attributes::NumberOfAttributes)
			return JNI_FALSE;
		int value = creature->getUnmodifiedMaxAttribute(static_cast<
			Attributes::Enumerator>(type));
		value += getIntField(*attrib, JavaLibrary::getFidAttributeValue());
		creature->setMaxAttribute(static_cast<Attributes::Enumerator>(type),
			static_cast<Attributes::Value>(value));
	}
	return JNI_TRUE;
}	// JavaLibrary::addToMaxAttribs

//========================================================================
// class JavaLibrary JNI hitpoint callback methods
//========================================================================

/**
 * Gets the hitpoints of an object.
 *
 * @param env		    Java environment
 * @param self		    class calling this function
 * @param target	    id of object
 *
 * @return the object's hitpoints, or ATTRIB_ERROR on error
 */
jint JNICALL ScriptMethodsAttributesNamespace::getHitpoints(JNIEnv *env, jobject self, jlong target)
{
	UNREF(self);

	const TangibleObject * object = nullptr;
	if (!JavaLibrary::getObject(target, object))
		return ATTRIB_ERROR;

	// make sure the object isn't a creature or is a vehicle
	if (object->asCreatureObject () != nullptr &&
		!GameObjectTypes::isTypeOf (object->getGameObjectType (), SharedObjectTemplate::GOT_vehicle))
		return ATTRIB_ERROR;

	jint hp = 0;
	hp = object->getMaxHitPoints() - object->getDamageTaken();
	return hp;
}	// JavaLibrary::getHitpoints

/**
 * Gets the max hitpoints of an object.
 *
 * @param env		    Java environment
 * @param self		    class calling this function
 * @param target	    id of object
 *
 * @return the object's max hitpoints, or ATTRIB_ERROR on error
 */
jint JNICALL ScriptMethodsAttributesNamespace::getMaxHitpoints(JNIEnv *env, jobject self, jlong target)
{
	UNREF(self);

	const TangibleObject * object = nullptr;
	if (!JavaLibrary::getObject(target, object))
		return ATTRIB_ERROR;

	// make sure the object isn't a creature or is a vehicle
	if (object->asCreatureObject () != nullptr &&
		!GameObjectTypes::isTypeOf (object->getGameObjectType (), SharedObjectTemplate::GOT_vehicle))
		return ATTRIB_ERROR;

	return object->getMaxHitPoints();
}	// JavaLibrary::getMaxHitpoints

/**
 * Gets the total hitpoints of an object, including component hitpoints.
 *
 * @param env		    Java environment
 * @param self		    class calling this function
 * @param target	    id of object
 *
 * @return the object's total hitpoints, or ATTRIB_ERROR on error
 */
jint JNICALL ScriptMethodsAttributesNamespace::getTotalHitpoints(JNIEnv *env, jobject self, jlong target)
{
	UNREF(self);

	const TangibleObject * object = nullptr;
	if (!JavaLibrary::getObject(target, object))
		return ATTRIB_ERROR;

	// make sure the object isn't a creature
	// make sure the object isn't a creature or is a vehicle
	if (object->asCreatureObject () != nullptr &&
		!GameObjectTypes::isTypeOf (object->getGameObjectType (), SharedObjectTemplate::GOT_vehicle))
		return ATTRIB_ERROR;

	return object->getMaxHitPoints();
}	// JavaLibrary::getTotalHitpoints

/**
 * Sets the hitpoints of an object. Note that if this is used to damage an
 * object it will bypass the object's armor!
 *
 * @param env		    Java environment
 * @param self		    class calling this function
 * @param target	    id of object
 * @param hitPoints		the hitpoints to set
 *
 * @return true on success, false on fail
 */
jboolean JNICALL ScriptMethodsAttributesNamespace::setHitpoints(JNIEnv *env, jobject self, jlong target, jint hitPoints)
{
	UNREF(self);

	TangibleObject * object = nullptr;
	if (!JavaLibrary::getObject(target, object))
		return JNI_FALSE;

	// make sure the object isn't a creature or is a vehicle
	if (object->asCreatureObject () != nullptr &&
		!GameObjectTypes::isTypeOf (object->getGameObjectType (), SharedObjectTemplate::GOT_vehicle))
		return JNI_FALSE;

	// determine how much "damage" is needed to get the object's hitpoints to the
	// right value
	int maxHitPoints = object->getMaxHitPoints();
	int currentHitPoints = maxHitPoints - object->getDamageTaken();
	if (hitPoints > maxHitPoints)
		hitPoints = maxHitPoints;

	CombatEngineData::DamageData damageData;
	struct AttribMod::AttribMod damage;
	damage.tag = 0;
	damage.attrib = Attributes::Health;
	damage.value = hitPoints - currentHitPoints;
	damage.attack = 0;
	damage.sustain = 0;
	damage.decay = AttribMod::AMDS_pool;
	damage.flags = AttribMod::AMF_directDamage;
	damageData.damage.push_back(damage);

	object->applyDamage(damageData);
	return JNI_TRUE;
}	// JavaLibrary::setHitpoints

/**
 * Sets the max hitpoints of an object.
 *
 * @param env		    Java environment
 * @param self		    class calling this function
 * @param target	    id of object
 * @param hitPoints		the hitpoints to set
 *
 * @return true on success, false on fail
 */
jboolean JNICALL ScriptMethodsAttributesNamespace::setMaxHitpoints(JNIEnv *env, jobject self, jlong target, jint hitPoints)
{
	UNREF(self);

	TangibleObject * object = nullptr;
	if (!JavaLibrary::getObject(target, object))
		return JNI_FALSE;

	// make sure the object isn't a creature or is a vehicle
	if (object->asCreatureObject () != nullptr &&
		!GameObjectTypes::isTypeOf (object->getGameObjectType (), SharedObjectTemplate::GOT_vehicle))
		return JNI_FALSE;

	object->setMaxHitPoints(hitPoints);
	return JNI_TRUE;
}	// JavaLibrary::setMaxHitpoints

/**
 * Sets the hitpoints of an object, ignoring the invulnerable flag of the object.
 * Note that if this is used to damage an object it will bypass the object's armor!
 *
 * @param env		    Java environment
 * @param self		    class calling this function
 * @param target	    id of object
 * @param hitPoints		the hitpoints to set
 *
 * @return true on success, false on fail
 */
jboolean JNICALL ScriptMethodsAttributesNamespace::setInvulnerableHitpoints(JNIEnv *env, jobject self, jlong target, jint hitPoints)
{
	UNREF(self);

	TangibleObject * object = nullptr;
	if (!JavaLibrary::getObject(target, object))
		return JNI_FALSE;

	// make sure the object isn't a creature or is a vehicle
	if (object->asCreatureObject () != nullptr &&
		!GameObjectTypes::isTypeOf (object->getGameObjectType (), SharedObjectTemplate::GOT_vehicle))
		return JNI_FALSE;

	// determine how much "damage" is needed to get the object's hitpoints to the
	// right value
	int maxHitPoints = object->getMaxHitPoints();
	int currentHitPoints = maxHitPoints - object->getDamageTaken();
	if (hitPoints > maxHitPoints)
		hitPoints = maxHitPoints;

	CombatEngineData::DamageData damageData;
	struct AttribMod::AttribMod damage;
	damage.tag = 0;
	damage.attrib = Attributes::Health;
	damage.value = hitPoints - currentHitPoints;
	damage.attack = 0;
	damage.sustain = 0;
	damage.decay = AttribMod::AMDS_pool;
	damage.flags = AttribMod::AMF_directDamage;
	damageData.damage.push_back(damage);
	damageData.ignoreInvulnerable = true;

	object->applyDamage(damageData);
	return JNI_TRUE;
}	// JavaLibrary::setInvulnerableHitpoints

/**
 * Returns the shock wound value of a creature.
 *
 * @param env		    Java environment
 * @param self		    class calling this function
 * @param target	    id of creature
 *
 * @return the shock wound value, or ATTRIB_ERROR on error
 */
jint JNICALL ScriptMethodsAttributesNamespace::getShockWound(JNIEnv *env, jobject self, jlong target)
{
	const CreatureObject * creature = nullptr;
	if (!JavaLibrary::getObject(target, creature))
		return ATTRIB_ERROR;

	return creature->getShockWounds();
}	// JavaLibrary::getShockWound

/**
 * Sets the shock wound value of a creature.
 *
 * @param env		    Java environment
 * @param self		    class calling this function
 * @param target	    id of creature
 * @param wound			the shock wound value
 *
 * @return true on success, false on fail
 */
jboolean JNICALL ScriptMethodsAttributesNamespace::setShockWound(JNIEnv *env, jobject self, jlong target, jint wound)
{
	CreatureObject * creature = nullptr;
	if (!JavaLibrary::getObject(target, creature))
		return JNI_FALSE;

	creature->setShockWounds(wound);
	return JNI_TRUE;
}	// JavaLibrary::setShockWound

/**
 * Adds to the shock wound value of a creature.
 *
 * @param env		    Java environment
 * @param self		    class calling this function
 * @param target	    id of creature
 * @param wound			the amount to add
 *
 * @return true on success, false on fail
 */
jboolean JNICALL ScriptMethodsAttributesNamespace::addShockWound(JNIEnv *env, jobject self, jlong target, jint wound)
{
	CreatureObject * creature = nullptr;
	if (!JavaLibrary::getObject(target, creature))
		return JNI_FALSE;

	creature->setShockWounds(creature->getShockWounds() + wound);
	return JNI_TRUE;
}	// JavaLibrary::addShockWound

/**
 * Heals the shock wound value of a creature.
 *
 * @param env		    Java environment
 * @param self		    class calling this function
 * @param target	    id of creature
 * @param value			the amount to heal
 *
 * @return true on success, false on fail
 */
jboolean JNICALL ScriptMethodsAttributesNamespace::healShockWound(JNIEnv *env, jobject self, jlong target, jint value)
{
	CreatureObject * creature = nullptr;
	if (!JavaLibrary::getObject(target, creature))
		return JNI_FALSE;

	creature->setShockWounds(creature->getShockWounds() - value);
	return JNI_TRUE;
}	// JavaLibrary::healShockWound

/**
 * Drains a creature's pool attributes, based on it's drain attributes.
 *
 * @param env		    Java environment
 * @param self		    class calling this function
 * @param target		creature we want to drain
 * @param action		amount to drain the creature's action attribute
 * @param mind			amount to drain the creature's mind attribute
 *
 * @return JNI_TRUE on success, JNI_FALSE on error
 */
jboolean JNICALL ScriptMethodsAttributesNamespace::drainAttributes(JNIEnv *env, jobject self, jlong target, jint action, jint mind)
{
	UNREF(self);

	CreatureObject * creature = 0;
	if (!JavaLibrary::getObject(target, creature))
		return JNI_FALSE;

	bool result = creature->drainAttributes(static_cast<Attributes::Value>(action), static_cast<Attributes::Value>(mind));

	if (result)
		return JNI_TRUE;
	return JNI_FALSE;
}	// JavaLibrary::drainAttributes

/**
 * Tests draining an attribute, but does not actually modify the attrib.
 *
 * @param env		    Java environment
 * @param self		    class calling this function
 * @param target		creature we want to drain
 * @param attribute		the attribute to test
 * @param value			the amount to drain the attribute by
 *
 * @return the amount the attribute would be drained by, or -1 on error
 */
jint JNICALL ScriptMethodsAttributesNamespace::testDrainAttribute(JNIEnv *env, jobject self, jlong target, jint attribute, jint value)
{
	UNREF(self);

	CreatureObject * creature = 0;
	if (!JavaLibrary::getObject(target, creature))
		return -1;

	int result = creature->testDrainAttribute(static_cast<Attributes::Enumerator>(
		attribute), static_cast<Attributes::Value>(value));

	return result;
}	// JavaLibrary::testDrainAttribute

/**
 * Sends a message to a player telling the ui to add a mod icon for a given
 * amount of time. The name passed to this function should be from the visible
 * attrib mod datatable.
 *
 * @param env		    Java environment
 * @param self		    class calling this function
 * @param target		player to send the info to
 * @param modName		the mod datatable entry name for the icon to show
 * @param time			how long to show the icon (<0 = indefinite)
 *
 * @return true on success, false on error
 */
jboolean JNICALL ScriptMethodsAttributesNamespace::addBuffIcon(JNIEnv *env, jobject self, jlong target, jstring modName, jfloat time)
{
	UNREF(self);

	JavaStringParam jmodName(modName);

	if (target == 0 || modName == 0)
		return JNI_FALSE;

	CreatureObject * creature = 0;
	if (!JavaLibrary::getObject(target, creature))
		return JNI_FALSE;

	std::string name;
	if (!JavaLibrary::convert(jmodName, name))
		return JNI_FALSE;

	creature->sendTimedModData(Crc::calculate(name.c_str()), time);
	return JNI_TRUE;
}	// JavaLibrary::addBuffIcon

/**
 * Sends a message to a player telling the ui to remove a mod icon. The name
 * passed to this function should be from the visible attrib mod datatable.
 *
 * @param env		    Java environment
 * @param self		    class calling this function
 * @param target		player to send the info to
 * @param modName		the mod datatable entry name for the icon to show
 *
 * @return true on success, false on error
 */
jboolean JNICALL ScriptMethodsAttributesNamespace::clearBuffIcon(JNIEnv *env, jobject self, jlong target, jstring modName)
{
	UNREF(self);

	JavaStringParam jmodName(modName);

	if (target == 0 || modName == 0)
		return JNI_FALSE;

	CreatureObject * creature = 0;
	if (!JavaLibrary::getObject(target, creature))
		return JNI_FALSE;

	std::string name;
	if (!JavaLibrary::convert(jmodName, name))
		return JNI_FALSE;

	creature->sendCancelTimedMod(Crc::calculate(name.c_str()));
	return JNI_TRUE;
}	// JavaLibrary::clearBuffIcon

/**
 * Returns the regeneration rate of the attribute of a creature.
 *
 * @param env		    Java environment
 * @param self		    class calling this function
 * @param target		player to send the info to
 * @param attrib		index of the attrib to get (health, action, mind)
 *
 * @return the regeneration rate, in units/sec
 */
jfloat JNICALL ScriptMethodsAttributesNamespace::getRegenRate(JNIEnv *env, jobject self, jlong target, jint attrib)
{
	const CreatureObject * creature = 0;
	if (!JavaLibrary::getObject(target, creature))
		return 0;

	return creature->getRegenRate(attrib);
}	// ScriptMethodsAttributesNamespace::getRegenRate

/**
 * Sets the regeneration rate of the attribute of a creature.
 *
 * @param env		    Java environment
 * @param self		    class calling this function
 * @param target		player to send the info to
 * @param attrib		index of the attrib to set (health, action, mind)
 * @param value			the regen rate, in units/sec
 */
void JNICALL ScriptMethodsAttributesNamespace::setRegenRate(JNIEnv *env, jobject self, jlong target, jint attrib, jfloat value)
{
	CreatureObject * creature = 0;
	if (!JavaLibrary::getObject(target, creature))
		return;

	creature->setRegenRate(attrib, value);
}	// ScriptMethodsAttributesNamespace::setRegenRate


