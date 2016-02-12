// ScriptMethodsWaypoint.cpp
// Copyright 2000-02, Sony Online Entertainment Inc., all rights reserved.
// Author: Justin Randall

//-----------------------------------------------------------------------

#include "serverScript/FirstServerScript.h"
#include "serverScript/JavaLibrary.h"

#include "serverGame/CreatureObject.h"
#include "serverGame/MissionObject.h"
#include "serverGame/ObjectIdManager.h"
#include "serverGame/PlayerCreatureController.h"
#include "serverGame/PlayerObject.h"
#include "serverGame/Region.h"
#include "serverGame/RegionMaster.h"
#include "serverGame/ServerWorld.h"
#include "sharedFoundation/TemporaryCrcString.h"
#include "sharedGame/Waypoint.h"
#include "sharedMathArchive/VectorArchive.h"


namespace ScriptMethodsWaypointNamespace
{
	NetworkId getWaypointCellIdFromTemplateNameAndCelllName(jstring const & buildingTemplate, jstring const & cellName)
	{
		JavaStringParam jbt(buildingTemplate);
		Unicode::String jbtn;
		if (!JavaLibrary::convert(jbt, jbtn))
		{
			DEBUG_WARNING(true, ("JavaLibrary::createWaypointInMissionInternal(): failed to buildingTemplate to a string"));
		}
		TemporaryCrcString crcBuilding(Unicode::wideToNarrow(jbtn).c_str(), true);
		uint64 buildingValue(crcBuilding.getCrc());
		buildingValue <<= static_cast<uint64>(32);
		
		JavaStringParam jcn(cellName);
		Unicode::String jcnb;
		if (!JavaLibrary::convert(jcn, jcnb))
		{
			DEBUG_WARNING(true, ("JavaLibrary::createWaypointInMissionInternal(): failed to cellName to a string"));
		}
		TemporaryCrcString crcCell(Unicode::wideToNarrow(jcnb).c_str(), true);
		uint64 cellValue(crcCell.getCrc());
		
		// rls - since the cell id is not used in the waypoint system,
		// we will combine the building and cell name crcs into the cell id.
		NetworkId const staticCellId(static_cast<NetworkId::NetworkIdType>(buildingValue | cellValue));

		return staticCellId;
	}

	bool install();

	jlong        JNICALL  createWaypointInDatapadFromLocation        (JNIEnv * env, jobject self, jlong target, jobject loc);
	jlong        JNICALL  createWaypointInMissionObjectFromLocation  (JNIEnv * env, jobject self, jlong target, jobject loc);
	jlong        JNICALL  createWaypointInDatapadFromObject          (JNIEnv * env, jobject self, jlong target, jlong obj);
	jlong        JNICALL  createWaypointInMissionObjectFromObject    (JNIEnv * env, jobject self, jlong target, jlong obj);
	jlong        JNICALL  createWaypointInDatapadWithLimitsFromLocation (JNIEnv * env, jobject self, jlong target, jobject obj);
	jlong        JNICALL  createWaypointInMissionInternal(JNIEnv * env, jobject self, jlong target, jobject jLoc, jstring buildingTemplate, jstring cellName);
	jlong        JNICALL  createWaypointInDatapadInternal(JNIEnv * env, jobject self, jlong target, jobject jLoc, jstring buildingTemplate, jstring cellName);
	void         JNICALL  destroyWaypointInDatapad    (JNIEnv * env, jobject self, jlong waypoint, jlong player);
	jboolean     JNICALL  getWaypointActive           (JNIEnv * env, jobject self, jlong waypoint);
	jobject      JNICALL  getWaypointLocation         (JNIEnv * env, jobject self, jlong waypoint);
	jstring      JNICALL  getWaypointName             (JNIEnv *, jobject, jlong waypoint);
	jlongArray   JNICALL  getWaypointsInDatapad       (JNIEnv *, jobject, jlong player);
	jobject      JNICALL  getWaypointRegion           (JNIEnv * env, jobject self, jlong waypoint);
	void         JNICALL  setWaypointActive           (JNIEnv * env, jobject self, jlong waypoint, jboolean isActive);
	void         JNICALL  setWaypointName             (JNIEnv * env, jobject, jlong waypoint, jstring name);
	void         JNICALL  setWaypointLocation         (JNIEnv * env, jobject self, jlong waypoint, jobject loc);
	jboolean     JNICALL  setWaypointColor            (JNIEnv * env, jobject self, jlong waypoint, jstring jscolor);
}

using namespace JNIWrappersNamespace;
using namespace ScriptMethodsWaypointNamespace;


//========================================================================
// install
//========================================================================

bool ScriptMethodsWaypointNamespace::install()
{
const JNINativeMethod NATIVES[] = {
	#define JF(a,b,c) {a,b,(void*)(ScriptMethodsWaypointNamespace::c)}
	JF("_createWaypointInDatapad",       "(JLscript/location;)J",  createWaypointInDatapadFromLocation),
	JF("_createWaypointInMissionObject", "(JLscript/location;)J",  createWaypointInMissionObjectFromLocation),
	JF("_createWaypointInDatapad",       "(JJ)J",    createWaypointInDatapadFromObject),
	JF("_createWaypointInMissionObject", "(JJ)J",    createWaypointInMissionObjectFromObject),
	JF("_createWaypointInDatapadWithLimits", "(JLscript/location;)J",  createWaypointInDatapadWithLimitsFromLocation),
	JF("_createWaypointInMissionInternal", "(JLscript/location;Ljava/lang/String;Ljava/lang/String;)J",  createWaypointInMissionInternal),
	JF("_createWaypointInDatapadInternal", "(JLscript/location;Ljava/lang/String;Ljava/lang/String;)J",  createWaypointInDatapadInternal),
	JF("_destroyWaypointInDatapad",      "(JJ)V",                  destroyWaypointInDatapad),
	JF("_getWaypointActive",    "(J)Z",           getWaypointActive),
	JF("_getWaypointsInDatapad", "(J)[J", getWaypointsInDatapad),
	JF("_getWaypointLocation",  "(J)Lscript/location;",  getWaypointLocation),
	JF("_getWaypointName",      "(J)Ljava/lang/String;", getWaypointName),
	JF("_getWaypointRegion",    "(J)Lscript/region;",    getWaypointRegion),
	JF("_setWaypointName",      "(JLjava/lang/String;)V", setWaypointName),
	JF("__setWaypointActiveNative",    "(JZ)V",          setWaypointActive),
	JF("__setWaypointLocationNative",  "(JLscript/location;)V",  setWaypointLocation),
	JF("__setWaypointColorNative",     "(JLjava/lang/String;)Z", setWaypointColor),
};

	return JavaLibrary::registerNatives(NATIVES, sizeof(NATIVES)/sizeof(NATIVES[0]));
}


//-----------------------------------------------------------------------

jlong JNICALL ScriptMethodsWaypointNamespace::createWaypointInDatapadFromLocation(JNIEnv * env, jobject self, jlong target, jobject jLoc)
{
	CreatureObject *targetCreature = 0;
	Location location;
	if (env && target && jLoc && JavaLibrary::getObject(target, targetCreature) && ScriptConversion::convert(LocalRefParam(jLoc), location))
	{
		PlayerObject * player = PlayerCreatureController::getPlayerObject(targetCreature);
		if(player)
		{
			Waypoint waypoint(player->createWaypoint(location, true));
			return (waypoint.getNetworkId()).getValue();
		}
	}
	return 0;
}

// ----------------------------------------------------------------------

jlong JNICALL ScriptMethodsWaypointNamespace::createWaypointInDatapadWithLimitsFromLocation(JNIEnv * env, jobject self, jlong target, jobject jLoc)
{
	CreatureObject *targetCreature = 0;
	Location location;
	if (env && target && jLoc && JavaLibrary::getObject(target, targetCreature) && ScriptConversion::convert(LocalRefParam(jLoc), location))
	{
		PlayerObject * player = PlayerCreatureController::getPlayerObject(targetCreature);
		if(player)
		{
			Waypoint waypoint(player->createWaypoint(location, false));
			if (waypoint.isValid())
				return (waypoint.getNetworkId()).getValue();
		}
	}
	return 0;
}

//-----------------------------------------------------------------------

jlong JNICALL ScriptMethodsWaypointNamespace::createWaypointInMissionObjectFromLocation(JNIEnv * env, jobject self, jlong target, jobject jLoc)
{
	MissionObject *targetMissionObj = 0;
	Location location;
	if (env && target && jLoc && JavaLibrary::getObject(target, targetMissionObj) && ScriptConversion::convert(LocalRefParam(jLoc), location))
	{
		Waypoint w = targetMissionObj->getWaypoint();
		w.setLocation(location);
		targetMissionObj->setWaypoint(w);
		return (w.getNetworkId()).getValue();
	}
	return 0;
}

//-----------------------------------------------------------------------

jlong JNICALL ScriptMethodsWaypointNamespace::createWaypointInDatapadFromObject(JNIEnv * env, jobject self, jlong target, jlong obj)
{
	CreatureObject *targetCreature = 0;
	ServerObject *s = 0;
	if (env && target && obj && JavaLibrary::getObject(target, targetCreature) && JavaLibrary::getObject(obj, s))
	{
		PlayerObject * player = PlayerCreatureController::getPlayerObject(targetCreature);
		if(player)
		{
			Waypoint w(player->createWaypoint(Location(s->getPosition_w(), NetworkId::cms_invalid, Location::getCrcBySceneName(ServerWorld::getSceneId())), true));
			w.setName(s->getAssignedObjectName());
			return (w.getNetworkId()).getValue();
		}
	}
	return 0;
}

//-----------------------------------------------------------------------

jlong JNICALL ScriptMethodsWaypointNamespace::createWaypointInMissionObjectFromObject(JNIEnv * env, jobject self, jlong target, jlong obj)
{
	MissionObject *targetMissionObj = 0;
	ServerObject *s = 0;
	if (env && target && obj && JavaLibrary::getObject(target, targetMissionObj) && JavaLibrary::getObject(obj, s))
	{
		Waypoint w = targetMissionObj->getWaypoint();
		w.setLocation(Location(s->getPosition_w(), NetworkId::cms_invalid, Location::getCrcBySceneName(ServerWorld::getSceneId())));
		w.setName(s->getAssignedObjectName());
		targetMissionObj->setWaypoint(w);
		return (w.getNetworkId()).getValue();
	}
	return 0;
}

//-----------------------------------------------------------------------

jlong JNICALL ScriptMethodsWaypointNamespace::createWaypointInMissionInternal(JNIEnv * env, jobject self, jlong target, jobject jLoc, jstring buildingTemplate, jstring cellName)
{
	MissionObject *targetMissionObj = 0;
	Location location;
	if (env && target && jLoc && JavaLibrary::getObject(target, targetMissionObj) && ScriptConversion::convert(LocalRefParam(jLoc), location))
	{
		Waypoint w = targetMissionObj->getWaypoint();

		NetworkId const & staticCellId = getWaypointCellIdFromTemplateNameAndCelllName(buildingTemplate, cellName);
		location.setCell(staticCellId);

		w.setLocation(location);
		targetMissionObj->setWaypoint(w);
		return (w.getNetworkId()).getValue();
	}
	return 0;
}

//-----------------------------------------------------------------------

jlong JNICALL ScriptMethodsWaypointNamespace::createWaypointInDatapadInternal(JNIEnv * env, jobject self, jlong target, jobject jLoc, jstring buildingTemplate, jstring cellName)
{
	CreatureObject *targetCreature = 0;
	Location location;
	if (env && target && jLoc && JavaLibrary::getObject(target, targetCreature) && ScriptConversion::convert(LocalRefParam(jLoc), location))
	{
		PlayerObject * player = PlayerCreatureController::getPlayerObject(targetCreature);
		if(player)
		{
			NetworkId const & staticCellId = getWaypointCellIdFromTemplateNameAndCelllName(buildingTemplate, cellName);
			location.setCell(staticCellId);

			Waypoint waypoint(player->createWaypoint(location, true));
			if (waypoint.isValid())
				return (waypoint.getNetworkId()).getValue();
		}
	}
	return 0;
}

//-----------------------------------------------------------------------

void JNICALL ScriptMethodsWaypointNamespace::destroyWaypointInDatapad(JNIEnv *, jobject, jlong waypoint, jlong player)
{
	CreatureObject * targetCreature = 0;
	if (JavaLibrary::getObject(player, targetCreature))
	{
		PlayerObject * p = PlayerCreatureController::getPlayerObject(targetCreature);
		if(p)
		{
			if(waypoint)
			{
				NetworkId id(waypoint);
				p->destroyWaypoint(id);
			}
		}
	}
}

//-----------------------------------------------------------------------

jboolean JNICALL ScriptMethodsWaypointNamespace::getWaypointActive(JNIEnv * env, jobject self, jlong waypoint)
{
	jboolean result = JNI_FALSE;
	if(env)
	{
		if(waypoint)
		{
			NetworkId id(waypoint);
			const Waypoint w = Waypoint::getWaypointById(id);
			if(w.isValid())
			{
				result = w.isActive();
			}
		}
	}

	return result;
}

//-----------------------------------------------------------------------

jobject JNICALL ScriptMethodsWaypointNamespace::getWaypointLocation(JNIEnv * env, jobject self, jlong waypoint)
{
	if(waypoint)
	{
		NetworkId id(waypoint);
		const Waypoint w = Waypoint::getWaypointById(id);
		if(w.isValid())
		{
			LocalRefPtr result;
			if (ScriptConversion::convert(w.getLocation(), result))
				return result->getReturnValue();
		}
	}
	return 0;
}

//-----------------------------------------------------------------------

jobject JNICALL ScriptMethodsWaypointNamespace::getWaypointRegion(JNIEnv * env, jobject self, jlong waypoint)
{
	if (waypoint != 0)
	{
		NetworkId id(waypoint);
		const Waypoint w = Waypoint::getWaypointById(id);
		if(w.isValid())
		{
			const Region * region = RegionMaster::getSmallestVisibleRegionAtPoint(w.getLocation().getSceneId(), w.getLocation().getCoordinates().x, w.getLocation().getCoordinates().z);
			if (region != nullptr)
			{
				LocalRefPtr result;
				if (ScriptConversion::convert(*region, result))
					return result->getReturnValue();
			}
		}
	}
	return 0;
}

//-----------------------------------------------------------------------

jlongArray JNICALL ScriptMethodsWaypointNamespace::getWaypointsInDatapad(JNIEnv *, jobject, jlong player)
{
	CreatureObject * creature = 0;
	if (JavaLibrary::getObject(player, creature))
	{
		PlayerObject * player = PlayerCreatureController::getPlayerObject(creature);
		if (player)
		{
			std::vector<NetworkId> waypoints;
			player->getWaypointsInDatapad(waypoints);
			LocalLongArrayRefPtr result;
			if (ScriptConversion::convert(waypoints, result))
				return result->getReturnValue();
		}
	}
	return 0;
}

//-----------------------------------------------------------------------

jstring JNICALL ScriptMethodsWaypointNamespace::getWaypointName(JNIEnv *, jobject, jlong waypoint)
{
	jstring result = 0;
	if(waypoint)
	{
		NetworkId id(waypoint);
		const Waypoint w = Waypoint::getWaypointById(id);
		if(w.isValid())
		{
			result = JavaString(w.getName()).getReturnValue();
		}
	}
	return result;
}

//-----------------------------------------------------------------------

void JNICALL ScriptMethodsWaypointNamespace::setWaypointActive(JNIEnv * env, jobject self, jlong waypoint, jboolean isActive)
{
	if(env)
	{
		if(waypoint)
		{
			NetworkId id(waypoint);
			Waypoint w = Waypoint::getWaypointById(id);
			if(w.isValid())
			{
				w.setActive(isActive == JNI_TRUE);
			}
		}
	}
}

//-----------------------------------------------------------------------

void JNICALL ScriptMethodsWaypointNamespace::setWaypointLocation(JNIEnv * env, jobject self, jlong waypoint, jobject jLoc)
{
	if(env)
	{
		if(waypoint)
		{
			if(jLoc)
			{
				NetworkId id(waypoint);
				Waypoint w = Waypoint::getWaypointById(id);
				if(w.isValid())
				{
					Location location;
					if(ScriptConversion::convert(LocalRefParam(jLoc), location))
					{
						w.setLocation(location);
					}
				}
			}
		}
	}
}

//-----------------------------------------------------------------------

void JNICALL ScriptMethodsWaypointNamespace::setWaypointName(JNIEnv *, jobject, jlong waypoint, jstring name)
{
	JavaStringParam localName(name);

	NetworkId id(waypoint);
	Waypoint w = Waypoint::getWaypointById(id);
	if(w.isValid())
	{
		Unicode::String     n;
		if (!JavaLibrary::convert (localName, n))
		{
			DEBUG_WARNING (true, ("setWaypointColor (): failed to convert name to string"));
		}
		else
		{
			w.setName(n);
		}
	}
}

//-----------------------------------------------------------------------

jboolean JNICALL ScriptMethodsWaypointNamespace::setWaypointColor(JNIEnv * env, jobject self, jlong waypoint, jstring jscolor)
{
	JavaStringParam localColor(jscolor);

	NetworkId id(waypoint);
	Waypoint w = Waypoint::getWaypointById(id);
	if(w.isValid())
	{
		std::string     color;
		if (!JavaLibrary::convert (localColor, color))
		{
			DEBUG_WARNING (true, ("setWaypointColor (): failed to convert color to string"));
			return JNI_FALSE;
		}
		w.setColor(Waypoint::getColorIdByName(color));
	}
	return JNI_TRUE;
}
