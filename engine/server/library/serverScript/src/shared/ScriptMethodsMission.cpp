// ScriptMethodsMission.cpp
// Copyright 2000-01, Sony Online Entertainment Inc., all rights reserved.
// Author: Justin Randall

//-----------------------------------------------------------------------

#include "serverScript/FirstServerScript.h"
#include "serverGame/ContainerInterface.h"
#include "serverGame/MissionObject.h"
#include "serverGame/PlanetObject.h"
#include "serverGame/Region.h"
#include "serverGame/RegionMaster.h"
#include "serverGame/ServerObject.h"
#include "serverGame/ServerUniverse.h"
#include "serverGame/ServerWorld.h"
#include "serverScript/JavaLibrary.h"
#include "sharedMath/Vector2d.h"
#include "sharedNetworkMessages/PopulateMissionBrowserMessage.h"
#include "sharedObject/ObjectTemplateList.h"

using namespace JNIWrappersNamespace;


// ======================================================================
// ScriptMethodsMissionNamespace
// ======================================================================

namespace ScriptMethodsMissionNamespace
{
	bool install();

	jlong        JNICALL createMissionObjectInCreatureMissionBag(JNIEnv * env, jobject self, jlong creature);
	void         JNICALL abortMission               (JNIEnv * env, jobject self, jlong missionObject);
	jobject      JNICALL assignMission              (JNIEnv * env, jobject self, jobject missionData, jobject playerCharacter);
	void         JNICALL endMission                 (JNIEnv * env, jobject self, jobject missionObject);
	jlong        JNICALL getMissionHolder           (JNIEnv * env, jobject self, jlong missionObject);
	jstring      JNICALL getMissionCreator          (JNIEnv * env, jobject self, jlong missionData);
	jobject      JNICALL getMissionDescription      (JNIEnv * env, jobject self, jlong missionData);
	jint         JNICALL getMissionDifficulty       (JNIEnv * env, jobject self, jlong missionData);
	jobject      JNICALL getMissionEndLocation      (JNIEnv * env, jobject self, jlong missionData);
	jint         JNICALL getMissionReward           (JNIEnv * env, jobject self, jlong missionData);
	jstring      JNICALL getMissionRootScriptName   (JNIEnv * env, jobject self, jlong missionData);
	jobject      JNICALL getMissionStartLocation    (JNIEnv * env, jobject self, jlong missionData);
	jint         JNICALL getMissionStatus           (JNIEnv * env, jobject self, jlong missionObject);
	jstring      JNICALL getMissionTargetName       (JNIEnv * env, jobject self, jlong missionData);
	jobject      JNICALL getMissionTitle            (JNIEnv * env, jobject self, jlong missionData);
	jstring      JNICALL getMissionType             (JNIEnv * env, jobject self, jlong missionData);
	void         JNICALL setMissionCreator          (JNIEnv * env, jobject self, jlong missionData, jstring creator);
	void         JNICALL setMissionDescription      (JNIEnv * env, jobject self, jlong missionData, jobject descriptionStringId);
	void         JNICALL setMissionDifficulty       (JNIEnv * env, jobject self, jlong missionData, jint difficulty);
	void         JNICALL setMissionEndLocation      (JNIEnv * env, jobject self, jlong missionData, jobject location);
	void         JNICALL setMissionReward           (JNIEnv * env, jobject self, jlong missionData, jint reward);
	void         JNICALL setMissionRootScriptName   (JNIEnv * env, jobject self, jlong missionData, jstring rootScriptName);
	void         JNICALL setMissionStartLocation    (JNIEnv * env, jobject self, jlong missionData, jobject location);
	void         JNICALL setMissionStatus           (JNIEnv * env, jobject self, jlong missionObject, jint newStatus);
	void         JNICALL startMission               (JNIEnv * env, jobject self, jlong creature, jlong mission);
	void         JNICALL setMissionTargetAppearance (JNIEnv * env, jobject self, jlong missionData, jstring targetAppearance);
	void         JNICALL setMissionTargetName       (JNIEnv * env, jobject self, jlong missionData, jstring targetName);
	void         JNICALL setMissionTitle            (JNIEnv * env, jobject self, jlong missionData, jobject titleStringId);
	void         JNICALL setMissionType             (JNIEnv * env, jobject self, jlong missionData, jstring typeName);
	jlongArray   JNICALL getMissionObjects          (JNIEnv * env, jobject self, jlong playerCharacter);
}


//========================================================================
// install
//========================================================================

bool ScriptMethodsMissionNamespace::install()
{
const JNINativeMethod NATIVES[] = {
	#define JF(a,b,c) {a,b,(void*)(ScriptMethodsMissionNamespace::c)}
	JF("_abortMission",                     "(J)V",                    abortMission),
	JF("_createMissionObjectInCreatureMissionBag", "(J)J",             createMissionObjectInCreatureMissionBag),
	JF("_getMissionStatus",                 "(J)I",                    getMissionStatus),
	JF("_setMissionStatus",                 "(JI)V",                   setMissionStatus),
	JF("_getMissionObjects",                "(J)[J",                   getMissionObjects),
	JF("_getMissionHolder",                 "(J)J",                    getMissionHolder),
	JF("_getMissionCreator",                "(J)Ljava/lang/String;",   getMissionCreator),
	JF("_getMissionDifficulty",             "(J)I",                    getMissionDifficulty),
	JF("_getMissionDescription",            "(J)Lscript/string_id;",   getMissionDescription),
	JF("_getMissionEndLocation",            "(J)Lscript/location;",    getMissionEndLocation),
	JF("_getMissionReward",                 "(J)I",                    getMissionReward),
	JF("_getMissionRootScriptName",         "(J)Ljava/lang/String;",   getMissionRootScriptName),
	JF("_getMissionStartLocation",          "(J)Lscript/location;",    getMissionStartLocation),
	JF("_getMissionTargetName",             "(J)Ljava/lang/String;",   getMissionTargetName),
	JF("_getMissionTitle",                  "(J)Lscript/string_id;",   getMissionTitle),
	JF("_getMissionType",                   "(J)Ljava/lang/String;",   getMissionType),
	JF("_setMissionCreator",                "(JLjava/lang/String;)V",  setMissionCreator),
	JF("_setMissionDescription",            "(JLscript/string_id;)V",  setMissionDescription),
	JF("_setMissionDifficulty",             "(JI)V",                   setMissionDifficulty),
	JF("_setMissionEndLocation",            "(JLscript/location;)V",   setMissionEndLocation),
	JF("_setMissionRootScriptName",         "(JLjava/lang/String;)V",  setMissionRootScriptName),
	JF("_setMissionReward",                 "(JI)V",                   setMissionReward),
	JF("_setMissionStartLocation",          "(JLscript/location;)V",   setMissionStartLocation),
	JF("_setMissionTitle",                  "(JLscript/string_id;)V",  setMissionTitle),
	JF("_setMissionTargetAppearance",       "(JLjava/lang/String;)V",  setMissionTargetAppearance),
	JF("_setMissionTargetName",             "(JLjava/lang/String;)V",  setMissionTargetName),
	JF("_setMissionType",                   "(JLjava/lang/String;)V",  setMissionType),
	JF("_startMission",                     "(JJ)V",                   startMission),
};

	return JavaLibrary::registerNatives(NATIVES, sizeof(NATIVES)/sizeof(NATIVES[0]));
}


//-----------------------------------------------------------------------

jlong JNICALL ScriptMethodsMissionNamespace::createMissionObjectInCreatureMissionBag(JNIEnv * env, jobject self, jlong creature)
{
	jlong result = 0;
	if(! env)
		return result;
	if(! self)
		return result;
	if(! creature)
		return result;

	CreatureObject * c = 0;
	if(JavaLibrary::getObject(creature, c))
	{
		ServerObject * missionBag = c->getMissionBag();
		if(missionBag)
		{
			Container * container = ContainerInterface::getContainer(*missionBag);
			ContainerIterator iter;
			for(iter = container->begin(); iter != container->end(); ++iter)
			{
				MissionObject * missionObject = safe_cast<MissionObject *>((*iter).getObject());
				if(missionObject)
				{
					result = (missionObject->getNetworkId()).getValue();
					break;
				}
			}
		}

		if(! result)
		{
			MissionObject * missionObject = MissionObject::createMissionObjectInCreatureMissionBag(c);
			if(missionObject)
			{
				result = (missionObject->getNetworkId()).getValue();
			}
		}
	}
	return result;
}

//-----------------------------------------------------------------------

void JNICALL ScriptMethodsMissionNamespace::abortMission(JNIEnv * env, jobject self, jlong missionObject)
{
	if(env)
	{
		if(self)
		{
			if(missionObject)
			{
				MissionObject * m;
				JavaLibrary::getObject(missionObject, m);
				if(m)
				{
					m->abortMission();
				}
				else
				{
					DEBUG_WARNING(true, ("[designer bug] abortMission() missionObject is invalid"));
				}
			}
			else
			{
				DEBUG_WARNING(true, ("[designer bug] abortMission() missionObject is nullptr"));
			}
		}
		else
		{
			DEBUG_WARNING(true, ("[programmer bug] abortMission() self is nullptr"));
		}
	}
	else
	{
		DEBUG_WARNING(true, ("[programmer bug] abortMission() JNIEnv is nullptr"));
	}
}

//-----------------------------------------------------------------------

void JNICALL ScriptMethodsMissionNamespace::endMission(JNIEnv * env, jobject self, jobject missionObject)
{
	if(env)
	{
		if(self)
		{
			if(missionObject)
			{
			}
		}
	}
}

//-----------------------------------------------------------------------

jlongArray JNICALL ScriptMethodsMissionNamespace::getMissionObjects(JNIEnv * env, jobject self, jlong playerCharacter)
{
	if (!env)
	{
		DEBUG_REPORT_LOG(true, ("No env found in getMissionObjects\n"));
		return 0;
	}
	if (!self)
	{
		DEBUG_REPORT_LOG(true, ("No self found in getMissionObjects\n"));
		return 0;
	}
	if (!playerCharacter)
	{
		DEBUG_REPORT_LOG(true, ("No PC found in getMissionObjects\n"));
		return 0;
	}

	CreatureObject * pc = 0;
	if(!JavaLibrary::getObject(playerCharacter, pc))
	{
		DEBUG_REPORT_LOG(true, ("Could not get object for PC in getMissionObjects\n"));
		return 0;
	}

	if(!pc)
	{
		DEBUG_REPORT_LOG(true, ("Could not resolve PC in getMissionObjects\n"));
		return 0;
	}

	const std::vector<CachedNetworkId> & missions = pc->getMissions();
	if(missions.empty())
	{
		DEBUG_REPORT_LOG(true, ("Mission list empty for getMissionObjects.\n"));
		return 0;
	}

	// build array
	int count = missions.size();
	LocalLongArrayRefPtr result = createNewLongArray(count);
	if (result != LocalLongArrayRef::cms_nullPtr)
	{
		int i;

		jlong jlongTmp;
		for(i = 0; i < count; ++i)
		{
			jlongTmp = missions[i].getValue();
			setLongArrayRegion(*result, i, 1, &jlongTmp);
		}
	}
	else
	{
		DEBUG_REPORT_LOG(true, ("Could not allocate new object array in getMissionObjects\n"));
	}
	return result->getReturnValue();
}

//-----------------------------------------------------------------------

jstring JNICALL ScriptMethodsMissionNamespace::getMissionCreator(JNIEnv * env, jobject self, jlong missionObject)
{
	jstring result = 0;
	if(env)
	{
		if(self)
		{
			if(missionObject)
			{
				MissionObject * mo = 0;
				if(JavaLibrary::getObject(missionObject, mo))
				{
					result = JavaString(mo->getMissionCreator()).getReturnValue();
				}
				else
				{
					DEBUG_WARNING(true, ("[designer bug] getMissionCreator() missionObject (parameter 1) is invalid"));
				}
			}
			else
			{
				DEBUG_WARNING(true, ("[designer bug] getMissionCreator() missionObject (parameter 1) is nullptr"));
			}
		}
		else
		{
			DEBUG_WARNING(true, ("[programmer bug] getMissionCreator() self is nullptr"));
		}
	}
	else
	{
		DEBUG_WARNING(true, ("[programmer bug] getMissionCreator() JNIEnv is nullptr"));
	}
	return result;
}

//-----------------------------------------------------------------------

jobject JNICALL ScriptMethodsMissionNamespace::getMissionDescription(JNIEnv * env, jobject self, jlong missionObject)
{
	if(! env)
	{
		DEBUG_WARNING(true, ("[programmer bug] getMissionDescription() JNIEnv is nullptr"));
		return 0;
	}

	if(! missionObject)
	{
		DEBUG_WARNING(true, ("[designer bug] getMissionDescription() missionObject (parameter 1) is nullptr"));
		return 0;
	}

	MissionObject * mo = 0;
	if(JavaLibrary::getObject(missionObject, mo))
	{
		LocalRefPtr result;
		const StringId & descriptionStringId = mo->getDescription();
		if (ScriptConversion::convert(descriptionStringId, result))
			return result->getReturnValue();
	}
	else
	{
		DEBUG_WARNING(true, ("[designer bug] getMissionDescription() missionObject (parameter 1) is invalid"));
	}
	return 0;
}

//-----------------------------------------------------------------------

jint JNICALL ScriptMethodsMissionNamespace::getMissionDifficulty(JNIEnv * env, jobject self, jlong missionObject)
{
	jint result = 0;
	if(env)
	{
		if(self)
		{
			if(missionObject)
			{
				MissionObject * mo = 0;
				if(JavaLibrary::getObject(missionObject, mo))
				{
					result = static_cast<jint>(mo->getDifficulty());
				}
				else
				{
					DEBUG_WARNING(true, ("[designer bug] getMissionDifficulty() missionObject (parameter 1) is invalid"));
				}
			}
			else
			{
				DEBUG_WARNING(true, ("[designer bug] getMissionDifficulty() missionObject (parameter 1) is nullptr"));
			}
		}
		else
		{
			DEBUG_WARNING(true, ("[programmer bug] getMissionDifficulty() self is nullptr"));
		}
	}
	else
	{
		DEBUG_WARNING(true, ("[programmer bug] getMissionDifficulty() JNIEnv is nullptr"));
	}
	return result;
}

//-----------------------------------------------------------------------

jobject JNICALL ScriptMethodsMissionNamespace::getMissionEndLocation(JNIEnv * env, jobject self, jlong missionObject)
{
	if(missionObject)
	{
		MissionObject * mo = 0;
		if(JavaLibrary::getObject(missionObject, mo))
		{
			Location loc = mo->getEndLocation();
			// @ todo : update location object in java to include planet, region and correct
			// area so it is a float describing a radius
			LocalRefPtr result;
			if (ScriptConversion::convert(loc.getCoordinates(), Location::getSceneNameByCrc(loc.getSceneIdCrc()), loc.getCell(), result))
				return result->getReturnValue();
		}
		else
		{
			DEBUG_WARNING(true, ("[designer bug] getMissionEndLocation() missionObject (parameter1) is invalid"));
		}
	}
	else
	{
		DEBUG_WARNING(true, ("[designer bug] getMissionEndLocation() missionObject (parameter1) is nullptr"));
	}
	return 0;
}

//-----------------------------------------------------------------------

jint JNICALL ScriptMethodsMissionNamespace::getMissionReward(JNIEnv * env, jobject self, jlong missionObject)
{
	jint result = 0;
	if(env)
	{
		if(self)
		{
			if(missionObject)
			{
				MissionObject * md = 0;
				if(JavaLibrary::getObject(missionObject, md))
				{
					result = md->getReward();
				}
				else
				{
					DEBUG_WARNING(true, ("[designer bug] getMissionReward() missionObject (parameter 1) is invalid"));
				}
			}
			else
			{
				DEBUG_WARNING(true, ("[designer bug] getMissionReward() missionObject (parameter 1) is nullptr"));
			}
		}
		else
		{
			DEBUG_WARNING(true, ("[programmer bug] getMissionReward() self is nullptr"));
		}
	}
	else
	{
		DEBUG_WARNING(true, ("[programmer bug] getMissionReward() JNIEnv is nullptr"));
	}
	return result;
}

//-----------------------------------------------------------------------

jobject JNICALL ScriptMethodsMissionNamespace::getMissionStartLocation(JNIEnv * env, jobject self, jlong missionObject)
{
	if(missionObject)
	{
		MissionObject * md = 0;
		if(JavaLibrary::getObject(missionObject, md))
		{
			LocalRefPtr result;
			const Location & loc = md->getStartLocation();
			if (ScriptConversion::convert(loc.getCoordinates(), Location::getSceneNameByCrc(loc.getSceneIdCrc()), loc.getCell(), result))
				return result->getReturnValue();
		}
		else
		{
			DEBUG_WARNING(true, ("[designer bug] getMissionStartLocation() missionObject (parameter 1) is invalid"));
		}
		}
	else
	{
		DEBUG_WARNING(true, ("[designer bug] getMissionStartLocation() missionObject (parameter 1) is nullptr"));
	}
	return 0;
}

//-----------------------------------------------------------------------

jstring JNICALL ScriptMethodsMissionNamespace::getMissionTargetName(JNIEnv * env, jobject self, jlong missionObject)
{
	UNREF(env);
	UNREF(self);

	if(missionObject)
	{
		MissionObject * md = 0;
		if(JavaLibrary::getObject(missionObject, md))
		{
			if(md)
			{
				return JavaString(md->getTargetName()).getReturnValue();
			}
		}
		else
		{
			DEBUG_WARNING(true, ("[designer bug] getMissionTargetName() missionObject (parameter 1) is invalid"));
		}
	}
	else
	{
		DEBUG_WARNING(true, ("[designer bug] getMissionTargetName() missionObject (parameter 1) is nullptr"));
	}
	return 0;
}

//-----------------------------------------------------------------------

jobject JNICALL ScriptMethodsMissionNamespace::getMissionTitle(JNIEnv * env, jobject self, jlong missionObject)
{
	if(! missionObject)
	{
		DEBUG_WARNING(true, ("[designer bug] getMissionTitle() missionObject (parameter 1) is nullptr"));
		return 0;
	}

	MissionObject * md = 0;
	if(JavaLibrary::getObject(missionObject, md))
	{
		LocalRefPtr result;
		const StringId & titleStringId = md->getTitle();
		if (ScriptConversion::convert(titleStringId, result))
			return result->getReturnValue();
	}
	else
	{
		DEBUG_WARNING(true, ("[designer bug] getMissionTitle() missionObject (parameter 1) is invalid"));
	}
	return 0;
}

//-----------------------------------------------------------------------

jstring JNICALL ScriptMethodsMissionNamespace::getMissionType(JNIEnv * env, jobject self, jlong missionObject)
{
	UNREF(env);
	UNREF(self);

	if(missionObject)
	{
		MissionObject * md = 0;
		if(JavaLibrary::getObject(missionObject, md))
		{
			if(md)
			{
				return JavaString(md->getMissionTypeString()).getReturnValue();
			}
		}
		else
		{
			DEBUG_WARNING(true, ("[designer bug] getMissionType() missionObject (parameter 1) is invalid"));
		}
	}
	else
	{
		DEBUG_WARNING(true, ("[designer bug] getMissionType() missionObject (parameter 1) is nullptr"));
	}
	return 0;
}

//-----------------------------------------------------------------------

jlong JNICALL ScriptMethodsMissionNamespace::getMissionHolder(JNIEnv * env, jobject self, jlong missionObject)
{
	jlong result = 0;
	if(env)
	{
		if(self)
		{
			if(missionObject)
			{
				MissionObject * mo = 0;
				if(JavaLibrary::getObject(missionObject, mo))
				{
					if (mo != nullptr && mo->getMissionHolderId() != NetworkId::cms_invalid)
					{
						result = (mo->getMissionHolderId()).getValue();
					}
					else
					{
						DEBUG_WARNING(true, ("[designer bug] getMissionHolder() missionObject (parameter 1) has no mission holder"));
					}
				}
				else
				{
					DEBUG_WARNING(true, ("[designer bug] getMissionHolder() missionObject (parameter 1) is invalid"));
				}
			}
			else
			{
				DEBUG_WARNING(true, ("[designer bug] getMissionHolder() missionObject (parameter 1) is nullptr"));
			}
		}
		else
		{
			DEBUG_WARNING(true, ("[programmer bug] getMissionHolder() self is nullptr"));
		}
	}
	else
	{
		DEBUG_WARNING(true, ("[programmer bug] getMissionHolder() JNIEnv is nullptr"));
	}
	return result;
}

//-----------------------------------------------------------------------

jstring JNICALL ScriptMethodsMissionNamespace::getMissionRootScriptName(JNIEnv * env, jobject self, jlong missionObject)
{
	UNREF(env);
	UNREF(self);

	if(missionObject)
	{
		MissionObject * md = 0;
		if(JavaLibrary::getObject(missionObject, md))
		{
			JavaString result(md->getRootScriptName().c_str());
			return result.getReturnValue();
		}
		else
		{
			DEBUG_WARNING(true, ("[designer bug] getMissionRootScriptName() missionObject (parameter 1) is invalid"));
		}
	}
	else
	{
		DEBUG_WARNING(true, ("[designer bug] getMissionRootScriptName() missionObject (parameter 1) is nullptr"));
	}
	return 0;
}

//-----------------------------------------------------------------------

void JNICALL ScriptMethodsMissionNamespace::setMissionCreator(JNIEnv *env, jobject self, jlong missionObject, jstring creator)
{
	if(env)
	{
		if(self)
		{
			if(missionObject)
			{
				if(creator)
				{
					MissionObject * md = 0;
					if(JavaLibrary::getObject(missionObject, md))
					{
						JavaString c(creator);
						Unicode::String s;
						if(JavaLibrary::convert(c, s))
							md->setMissionCreator(s);
					}
					else
					{
						DEBUG_WARNING(true, ("[designer bug] setMissionCreator() missionObject (parameter 1) is invalid"));
					}
				}
				else
				{
					DEBUG_WARNING(true, ("[designer bug] setMissionCreator() creator (parameter 2) is nullptr"));
				}
			}
			else
			{
				DEBUG_WARNING(true, ("[designer bug] setMissionCreator() missionObject (parameter 1) is nullptr"));
			}
		}
		else
		{
			DEBUG_WARNING(true, ("[programmer bug] setMissionCreator() self is nullptr"));
		}
	}
	else
	{
		DEBUG_WARNING(true, ("[programmer bug] setMissionCreator() JNIEnv is nullptr"));
	}
}

//-----------------------------------------------------------------------

void JNICALL ScriptMethodsMissionNamespace::setMissionDescription(JNIEnv * env, jobject self, jlong missionObject, jobject descriptionStringId)
{
	if(missionObject)
	{
		MissionObject * md = 0;
		if(JavaLibrary::getObject(missionObject, md))
		{
			StringId desc;
			if(ScriptConversion::convert(descriptionStringId, desc))
			{
				md->setDescription(desc);
			}
			else
			{
				DEBUG_WARNING(true, ("[designer bug] setMissionDescription() descriptionStringId (parameter 2) is invalid"));
			}
		}
		else
		{
			DEBUG_WARNING(true, ("[designer bug] setMissionDescription() missionObject (parameter 1) is invalid"));
		}
	}
	else
	{
		DEBUG_WARNING(true, ("[designer bug] setMissionDescription() missionObject (parameter 1) is nullptr"));
	}
}

//-----------------------------------------------------------------------

void JNICALL ScriptMethodsMissionNamespace::setMissionDifficulty(JNIEnv * env, jobject self, jlong missionObject, jint difficulty)
{
	if(missionObject)
	{
		MissionObject * md = 0;
		if(JavaLibrary::getObject(missionObject, md))
		{
			md->setDifficulty(static_cast<int>(difficulty));
		}
		else
		{
			DEBUG_WARNING(true, ("[designer bug] setMissionDifficulty() missionObject (parameter 1) is invalid"));
		}
	}
	else
	{
		DEBUG_WARNING(true, ("[designer bug] setMissionDifficulty() missionObject (parameter 1) is nullptr"));
	}
}

//-----------------------------------------------------------------------

void JNICALL ScriptMethodsMissionNamespace::setMissionEndLocation(JNIEnv * env, jobject self, jlong missionObject, jobject location)
{
	if(env)
	{
		if(self)
		{
			if(missionObject)
			{
				MissionObject * md = 0;
				if(JavaLibrary::getObject(missionObject, md))
				{
					// @todo : flesh out java location class and set this appropriately!
					JavaStringPtr p = getStringField(LocalRefParam(location), JavaLibrary::getFidLocationArea());
					if (p != JavaString::cms_nullPtr)
					{
						std::string s;
						if (JavaLibrary::convert(*p, s))
						{
							Location loc(Vector(env->GetFloatField(location, JavaLibrary::getFidLocationX()), env->GetFloatField(location, JavaLibrary::getFidLocationY()), env->GetFloatField(location, JavaLibrary::getFidLocationZ())), JavaLibrary::getNetworkId(env->GetObjectField(location, JavaLibrary::getFidLocationCell())), Location::getCrcBySceneName(s.c_str()));
							md->setEndLocation(loc);
						}
					}
				}
			}
		}
	}
}

//-----------------------------------------------------------------------

void JNICALL ScriptMethodsMissionNamespace::setMissionReward(JNIEnv * env, jobject self, jlong missionObject, jint reward)
{
	if(env)
	{
		if(self)
		{
			if(missionObject)
			{
				MissionObject * md = 0;
				if(JavaLibrary::getObject(missionObject, md))
				{
					md->setReward(static_cast<int>(reward));
				}
				else
				{
					DEBUG_WARNING(true, ("[designer bug] setMissionProfessionRank() missionObject (parameter 1) is invalid"));
				}
			}
			else
			{
				DEBUG_WARNING(true, ("[designer bug] setMissionProfessionRank() missionObject (parameter 1) is nullptr"));
			}
		}
		else
		{
			DEBUG_WARNING(true, ("[programmer bug] setMissionProfessionRank() self is nullptr"));
		}
	}
	else
	{
		DEBUG_WARNING(true, ("[programmer bug] setMissionProfessionRank() JNIEnv is nullptr"));
	}
}

//-----------------------------------------------------------------------

void JNICALL ScriptMethodsMissionNamespace::setMissionStartLocation(JNIEnv * env, jobject self, jlong missionObject, jobject location)
{
	if(env)
	{
		if(self)
		{
			if(missionObject)
			{
				MissionObject * md = 0;
				if(JavaLibrary::getObject(missionObject, md))
				{
					if (location)
					{
						JavaStringPtr p = getStringField(LocalRefParam(location), JavaLibrary::getFidLocationArea());
						if (p != JavaString::cms_nullPtr)
						{
							std::string s;
							if (JavaLibrary::convert(*p, s))
							{
								Location loc(Vector(env->GetFloatField(location, JavaLibrary::getFidLocationX()), env->GetFloatField(location, JavaLibrary::getFidLocationY()), env->GetFloatField(location, JavaLibrary::getFidLocationZ())), JavaLibrary::getNetworkId(env->GetObjectField(location, JavaLibrary::getFidLocationCell())), Location::getCrcBySceneName(s.c_str()));
								md->setStartLocation(loc);
							}
						}
					}
					else
					{
						DEBUG_WARNING(true, ("[designer bug] setMissionStartLocation() location (parameter 2) is nullptr"));
					}
				}
				else
				{
					DEBUG_WARNING(true, ("[designer bug] setMissionStartLocation() missionObject (parameter 1) is invalid"));
				}
			}
			else
			{
				DEBUG_WARNING(true, ("[designer bug] setMissionStartLocation() missionObject (parameter 1) is nullptr"));
			}
		}
		else
		{
			DEBUG_WARNING(true, ("[programmer bug] setMissionStartLocation() self is nullptr"));
		}
	}
	else
	{
		DEBUG_WARNING(true, ("[programmer bug] setMissionStartLocation() JNIEnv is nullptr"));
	}
}

//-----------------------------------------------------------------------

void JNICALL ScriptMethodsMissionNamespace::setMissionTargetAppearance(JNIEnv * env, jobject self, jlong missionObject, jstring appearanceName)
{
	if(env)
	{
		if(self)
		{
			if(missionObject)
			{
				MissionObject * md = 0;
				if(JavaLibrary::getObject(missionObject, md))
				{
					std::string s;
					if(JavaLibrary::convert(JavaString(appearanceName), s))
					{
						md->setTargetAppearance(s);
					}
				}
				else
				{
					DEBUG_WARNING(true, ("[designer bug] setMissionTarget() missionObject (parameter 1) is invalid"));
				}
			}
			else
			{
				DEBUG_WARNING(true, ("[designer bug] setMissionTarget() missionObject (parameter 1) is nullptr"));
			}
		}
		else
		{
			DEBUG_WARNING(true, ("[programmer bug] setMissionTarget() self is nullptr"));
		}
	}
	else
	{
		DEBUG_WARNING(true, ("[programmer bug] setMissionTarget() JNIEnv is nullptr"));
	}
}

//-----------------------------------------------------------------------

void JNICALL ScriptMethodsMissionNamespace::setMissionTargetName(JNIEnv * env, jobject self, jlong missionObject, jstring name)
{
	if(env)
	{
		if(self)
		{
			if(missionObject)
			{
				MissionObject * md = 0;
				if(JavaLibrary::getObject(missionObject, md))
				{
					std::string desc;
					if(JavaLibrary::convert(JavaString(name), desc))
					{
						md->setTargetName(desc);
					}
				}
				else
				{
					DEBUG_WARNING(true, ("[designer bug] setMissionTarget() missionObject (parameter 1) is invalid"));
				}
			}
			else
			{
				DEBUG_WARNING(true, ("[designer bug] setMissionTarget() missionObject (parameter 1) is nullptr"));
			}
		}
		else
		{
			DEBUG_WARNING(true, ("[programmer bug] setMissionTarget() self is nullptr"));
		}
	}
	else
	{
		DEBUG_WARNING(true, ("[programmer bug] setMissionTarget() JNIEnv is nullptr"));
	}
}

//-----------------------------------------------------------------------

void JNICALL ScriptMethodsMissionNamespace::setMissionTitle(JNIEnv * env, jobject self, jlong missionObject, jobject titleStringId)
{
	if(missionObject)
	{
		MissionObject * md = 0;
		if(JavaLibrary::getObject(missionObject, md))
		{
			StringId title;
			if(ScriptConversion::convert(titleStringId, title))
			{
				md->setTitle(title);
			}
			else
			{
				DEBUG_WARNING(true, ("[designer bug] setMissionTitle() titleStringId (parameter 2) is invalid"));
			}
		}
		else
		{
			DEBUG_WARNING(true, ("[designer bug] setMissionTitle() missionObject (parameter 1) is invalid"));
		}
	}
	else
	{
		DEBUG_WARNING(true, ("[designer bug] setMissionTitle() missionObject (parameter 1) is nullptr"));
	}
}

//-----------------------------------------------------------------------

void JNICALL ScriptMethodsMissionNamespace::setMissionType(JNIEnv *env, jobject self, jlong missionObject, jstring typeName)
{
	if(env)
	{
		if(self)
		{
			if(missionObject)
			{
				if(typeName)
				{
					MissionObject * md = 0;
					if(JavaLibrary::getObject(missionObject, md))
					{
						JavaStringParam localTypeName(typeName);
						std::string n;
						if(JavaLibrary::convert(localTypeName, n))
						{
							md->setMissionType(n);
						}
					}
					else
					{
						DEBUG_WARNING(true, ("[designer bug] setMissionType() missionObject (parameter 1) is invalid"));
					}
				}
				else
				{
					DEBUG_WARNING(true, ("[designer bug] setMissionType() typeName (parameter 2) is nullptr"));
				}
			}
			else
			{
				DEBUG_WARNING(true, ("[designer bug] setMissionType() missionObject (parameter 1) is nullptr"));
			}
		}
		else
		{
			DEBUG_WARNING(true, ("[programmer bug] setMissionType() self is nullptr"));
		}
	}
	else
	{
		DEBUG_WARNING(true, ("[programmer bug] setMissionType() JNIEnv is nullptr"));
	}
}

//-----------------------------------------------------------------------

void JNICALL ScriptMethodsMissionNamespace::setMissionRootScriptName(JNIEnv *env, jobject self, jlong missionObject, jstring rootScriptName)
{
	JavaStringParam localRootScriptName(rootScriptName);

	if(env)
	{
		if(self)
		{
			if(missionObject)
			{
				if(rootScriptName)
				{
					MissionObject * md = 0;
					if(JavaLibrary::getObject(missionObject, md))
					{
						std::string n;
						if(JavaLibrary::convert(localRootScriptName, n))
						{
							md->setRootScriptName(n);
						}
						else
						{
							DEBUG_WARNING(true, ("[designer bug] setMissionRootScriptName() rootScriptName (parameter 2) is invalid"));
						}
					}
					else
					{
						DEBUG_WARNING(true, ("[designer bug] setMissionRootScriptName() missionObject (parameter 1) is invalid"));
					}
				}
				else
				{
					DEBUG_WARNING(true, ("[designer bug] setMissionRootScriptName() rootScriptName (parameter 2) is nullptr"));
				}
			}
			else
			{
				DEBUG_WARNING(true, ("[designer bug] setMissionRootScriptName() missionObject (parameter 1) is nullptr"));
			}
		}
		else
		{
			DEBUG_WARNING(true, ("[programmer bug] setMissionRootScriptName() self is nullptr"));
		}
	}
	else
	{
		DEBUG_WARNING(true, ("[programmer bug] setMissionRootScriptName() JNIEnv is nullptr"));
	}
}

//-----------------------------------------------------------------------

jint JNICALL ScriptMethodsMissionNamespace::getMissionStatus(JNIEnv * env, jobject self, jlong missionObject)
{
	jint result = 0;
	if(! env)
		return 0;
	if(! self)
		return 0;

	MissionObject * mo = 0;
	if(JavaLibrary::getObject(missionObject, mo))
	{
		result = mo->getStatus();
	}
	return result;

}

//-----------------------------------------------------------------------

void JNICALL ScriptMethodsMissionNamespace::setMissionStatus(JNIEnv * env, jobject self, jlong missionObject, jint status)
{
	if(! env)
		return;

	if(! self)
		return;

	MissionObject * mo = 0;
	if(JavaLibrary::getObject(missionObject, mo))
	{
		mo->setStatus(status);
	}
}

//-----------------------------------------------------------------------

void JNICALL ScriptMethodsMissionNamespace::startMission(JNIEnv * env, jobject self, jlong creature, jlong mission)
{
	if(! env)
		return;

	if(! self)
		return;

	MissionObject * mo = 0;
	if(JavaLibrary::getObject(mission, mo))
	{
		CreatureObject * co = 0;
		if(JavaLibrary::getObject(creature, co))
		{
			co->assignMission(mo);
		}
	}
}

//-----------------------------------------------------------------------

