// ScriptMethodsChat.cpp
// Copyright 2000-02, Sony Online Entertainment Inc., all rights reserved.
// Author: Justin Randall

//-----------------------------------------------------------------------

#include "serverScript/FirstServerScript.h"
#include "serverScript/JavaLibrary.h"

#include "UnicodeUtils.h"
#include "serverGame/Chat.h"
#include "serverGame/GameServer.h"
#include "serverGame/ServerObject.h"
#include "serverGame/ServerWorld.h"
#include "serverNetworkMessages/FactionalSystemMessage.h"
#include "serverScript/ConfigServerScript.h"
#include "sharedGame/OutOfBandPackager.h"
#include "sharedGame/ProsePackage.h"
#include "sharedGame/ProsePackageArchive.h"
#include "sharedGame/SocialsManager.h"
#include "sharedGame/Waypoint.h"
#include "sharedGame/WaypointData.h"
#include "sharedMathArchive/VectorArchive.h"

// ======================================================================
// ScriptMethodsChatNamespace
// ======================================================================

namespace ScriptMethodsChatNamespace
{
	bool install();

	void         JNICALL chatCreateRoom(JNIEnv * env, jobject self, jstring me, jboolean isPublic, jstring roomName, jstring roomTitle);
	void         JNICALL chatDestroyRoom(JNIEnv * env, jobject self, jstring roomName);
	void         JNICALL chatEnterRoom(JNIEnv * env, jobject self, jstring who, jstring roomName);
	void         JNICALL chatExitRoom(JNIEnv * env, jobject self, jstring who, jstring roomName);
	void         JNICALL chatAddModeratorToRoom(JNIEnv * env, jobject self, jstring roomName, jstring moderatorName);
	jstring      JNICALL chatPackOutOfBandToken(JNIEnv * env, jobject self, jlong source, jstring target, jint position);
	jstring      JNICALL chatPackOutOfBandWaypoint(JNIEnv * env, jobject self, jlong source, jstring target, jint position);
	jstring      JNICALL chatPackOutOfBandWaypointData(JNIEnv * env, jobject self, jstring target, jint position, jstring planet, jfloat x, jfloat z, jobject nameId, jstring name);
	jstring      JNICALL packOutOfBandProsePackage(JNIEnv * env, jobject self, jstring target, jint position, jobject stringId, jlong objActor, jlong objTarget, jlong objOther, jobject nameIdActor, jobject nameIdTarget, jobject nameIdOther, jstring nameActor, jstring nameTarget, jstring nameOther, jint digitInteger, jfloat digitFloat, jboolean complexGrammar);
	void         JNICALL chatRemoveModeratorFromRoom(JNIEnv * env, jobject self, jstring roomName, jstring moderatorName);
	void         JNICALL chatSendInstantMessage(JNIEnv * env, jobject self, jstring from, jstring to, jstring messageText, jstring outOfBand);
	void         JNICALL chatSendPersistentMessage(JNIEnv * env, jobject self, jstring from, jstring to, jstring messageText, jstring outOfBand, jstring subject);
	void         JNICALL chatSendToRoom(JNIEnv * env, jobject self, jstring from, jstring roomName, jstring messageText, jstring outOfBand);
	void         JNICALL chatSendSystemMessage(JNIEnv * env, jobject self, jstring to, jstring message, jstring oob);
	void         JNICALL chatSendSystemMessageObjId(JNIEnv * env, jobject self, jlong to, jstring message, jstring oob);
	jstring      JNICALL getEmoteFromCrc(JNIEnv * env, jobject self, jint crc);
	void         JNICALL chatSendQuestSystemMessage(JNIEnv * env, jobject self, jlong to, jstring message, jstring oob);
	void         JNICALL chatSendFactionalSystemMessagePlanet(JNIEnv * env, jobject self, jstring prosePackage, jobject loc, jfloat radius, jboolean notifyImperial, jboolean notifyRebel);
}


//========================================================================
// install
//========================================================================

bool ScriptMethodsChatNamespace::install()
{
const JNINativeMethod NATIVES[] = {
	#define JF(a,b,c) {a,b,(void*)(ScriptMethodsChatNamespace::c)}
	JF("_chatCreateRoom", "(Ljava/lang/String;ZLjava/lang/String;Ljava/lang/String;)V", chatCreateRoom),
	JF("chatDestroyRoom", "(Ljava/lang/String;)V", chatDestroyRoom),
	JF("_chatEnterRoom", "(Ljava/lang/String;Ljava/lang/String;)V", chatEnterRoom),
	JF("_chatExitRoom", "(Ljava/lang/String;Ljava/lang/String;)V", chatExitRoom),
	JF("chatAddModeratorToRoom", "(Ljava/lang/String;Ljava/lang/String;)V", chatAddModeratorToRoom),
	JF("chatRemoveModeratorFromRoom", "(Ljava/lang/String;Ljava/lang/String;)V", chatRemoveModeratorFromRoom),
	JF("_chatSendInstantMessage", "(Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;)V", chatSendInstantMessage),
	JF("_chatSendPersistentMessage", "(Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;)V", chatSendPersistentMessage),
	JF("_chatSendToRoom", "(Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;)V", chatSendToRoom),
	JF("__packOutOfBandToken", "(JLjava/lang/String;I)Ljava/lang/String;", chatPackOutOfBandToken),
	JF("__packOutOfBandWaypoint", "(JLjava/lang/String;I)Ljava/lang/String;", chatPackOutOfBandWaypoint),
	JF("_packOutOfBandWaypointData", "(Ljava/lang/String;ILjava/lang/String;FFLscript/string_id;Ljava/lang/String;)Ljava/lang/String;", chatPackOutOfBandWaypointData),
	JF("__packOutOfBandProsePackage", "(Ljava/lang/String;ILscript/string_id;JJJLscript/string_id;Lscript/string_id;Lscript/string_id;Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;IFZ)Ljava/lang/String;", packOutOfBandProsePackage),
	JF("sendSystemMessage", "(Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;)V", chatSendSystemMessage),
	JF("_sendSystemMessage", "(JLjava/lang/String;Ljava/lang/String;)V", chatSendSystemMessageObjId),
	JF("getEmoteFromCrc", "(I)Ljava/lang/String;", getEmoteFromCrc),
	JF("_sendQuestSystemMessage", "(JLjava/lang/String;Ljava/lang/String;)V", chatSendQuestSystemMessage),
	JF("sendFactionalSystemMessagePlanet", "(Ljava/lang/String;Lscript/location;FZZ)V", chatSendFactionalSystemMessagePlanet),
};

	return JavaLibrary::registerNatives(NATIVES, sizeof(NATIVES)/sizeof(NATIVES[0]));
}


//-----------------------------------------------------------------------

void JNICALL ScriptMethodsChatNamespace::chatAddModeratorToRoom(JNIEnv * env, jobject self, jstring roomName, jstring moderatorName)
{
	JavaStringParam jsprn(roomName);
	std::string r;
	if(! JavaLibrary::convert(jsprn, r))
		return;

	JavaStringParam jspmn(moderatorName);
	std::string m;
	if(! JavaLibrary::convert(jspmn, m))
		return;

	Chat::addModeratorToRoom(m, r);
}

//-----------------------------------------------------------------------

void JNICALL JNICALL ScriptMethodsChatNamespace::chatCreateRoom(JNIEnv * env, jobject self, jstring me, jboolean isPublic, jstring roomName, jstring roomTitle)
{
	JavaStringParam jspMe(me);
	std::string m;
	if(!JavaLibrary::convert(jspMe, m))
		return;

	JavaStringParam jsprn(roomName);
	std::string r;
	if(! JavaLibrary::convert(jsprn, r))
		return;

	JavaStringParam jspt(roomTitle);
	std::string t;
	if(! JavaLibrary::convert(jspt, t))
		return;

	Chat::createRoom(m, isPublic == JNI_TRUE, r, t);
}

//-----------------------------------------------------------------------

void JNICALL JNICALL ScriptMethodsChatNamespace::chatDestroyRoom(JNIEnv * env, jobject self, jstring roomName)
{
	JavaStringParam jsprn(roomName);
	std::string r;
	if(! JavaLibrary::convert(jsprn, r))
		return;

	Chat::destroyRoom(r);

}

//-----------------------------------------------------------------------

void JNICALL ScriptMethodsChatNamespace::chatEnterRoom(JNIEnv * env, jobject self, jstring me, jstring roomName)
{
	JavaStringParam jspm(me);
	std::string m;
	if(! JavaLibrary::convert(jspm, m))
		return;

	JavaStringParam jspr(roomName);
	std::string r;
	if(! JavaLibrary::convert(jspr, r))
		return;

	Chat::enterRoom(m, r, true, false);
}

//-----------------------------------------------------------------------

void JNICALL ScriptMethodsChatNamespace::chatExitRoom(JNIEnv * env, jobject self, jstring me, jstring roomName)
{
	JavaStringParam jspm(me);
	std::string m;
	if(! JavaLibrary::convert(jspm, m))
		return;

	JavaStringParam jspr(roomName);
	std::string r;
	if(! JavaLibrary::convert(jspr, r))
		return;

	Chat::exitRoom(m, r);
}

//-----------------------------------------------------------------------

jstring JNICALL ScriptMethodsChatNamespace::chatPackOutOfBandToken(JNIEnv * env, jobject self, jlong source, jstring target, jint position)
{
	UNREF(env);
	UNREF(self);
	UNREF(source);
	UNREF(target);
	UNREF(position);

	JavaString result("JavaLibrary::chatPackOutOfBandToken - FAILED Token archive not implemented");
	return result.getReturnValue();
}

//----------------------------------------------------------------------

jstring JNICALL ScriptMethodsChatNamespace::chatPackOutOfBandWaypointData(JNIEnv * env, jobject self, jstring target, jint position, jstring planet, jfloat x, jfloat z, jobject nameId, jstring name)
{
	UNREF(env);
	UNREF(self);

	Unicode::String t;   //target
	std::string     p;   //planet
	Unicode::String n;   //name
	StringId        nid; //nameId

	//-- target may be nullptr, we'll just start a new string
	if (target)
	{
		const JavaStringParam jtarget(target);
		if (!JavaLibrary::convert(jtarget, t))
		{
			DEBUG_WARNING (true, ("ScriptMethodsChat JavaLibrary::chatPackOutOfBandWaypointData failed bad target string"));
			return 0;
		}
	}

	//-- if planet is nullptr, just use the current sceneId
	if (planet)
	{
		const JavaStringParam jplanet(planet);
		Unicode::String p_tmp;
		if (!JavaLibrary::convert(jplanet, p_tmp))
		{
			DEBUG_WARNING (true, ("ScriptMethodsChat JavaLibrary::chatPackOutOfBandWaypointData failed bad planet string"));
			return 0;
		}

		p = Unicode::wideToNarrow (p_tmp);
	}
	else
	{
		p = ServerWorld::getSceneId ();
	}

	if (!name && !nameId)
	{
		DEBUG_WARNING (true, ("ScriptMethodsChat JavaLibrary::chatPackOutOfBandWaypointData failed bad nameId"));
		return 0;

	}

	if (nameId)
	{
		if (!ScriptConversion::convert (nameId, nid))
		{
			DEBUG_WARNING (true, ("ScriptMethodsChat JavaLibrary::chatPackOutOfBandWaypointData failed bad nameId"));
			return 0;
		}
	}

	if (name)
	{
		const JavaStringParam jname (name);
		if (!JavaLibrary::convert(jname, n))
		{
			DEBUG_WARNING (true, ("ScriptMethodsChat JavaLibrary::chatPackOutOfBandWaypointData failed bad name string"));
			return 0;
		}
	}

	WaypointDataBase wd;
	wd.m_location = Location(Vector(x, 0, z), NetworkId::cms_invalid, Location::getCrcBySceneName(p));
	wd.setName(n);

	OutOfBandPackager::pack (wd, position, t);

	JavaString result(t);
	return result.getReturnValue();
}

//-----------------------------------------------------------------------

jstring JNICALL ScriptMethodsChatNamespace::chatPackOutOfBandWaypoint(JNIEnv * env, jobject self, jlong source, jstring target, jint position)
{
	UNREF(env);
	UNREF(self);

	Unicode::String t;

	if (target)
	{
		const JavaStringParam jt(target);
		if (!JavaLibrary::convert(jt, t))
		{
			DEBUG_WARNING (true, ("ScriptMethodsChat JavaLibrary::chatPackOutOfBandWaypoint failed bad target string"));
			return 0;
		}
	}

	if (!source)
	{
		DEBUG_WARNING (true, ("ScriptMethodsChat JavaLibrary::chatPackOutOfBandWaypoint failed nullptr source"));
		return 0;
	}

	NetworkId id(source);
	const Waypoint waypoint = Waypoint::getWaypointById(id);
	if (waypoint.isValid())
	{
		OutOfBandPackager::pack (waypoint.getWaypointDataBase(), position, t);
	}
	else
	{
		ServerObject * obj = 0;
		if (JavaLibrary::getObject(source, obj) && obj)
		{
			WaypointDataBase wd;
			wd.m_location = Location(obj->getPosition_w(), NetworkId::cms_invalid, Location::getCrcBySceneName(ServerWorld::getSceneId()));
			wd.setName(obj->getAssignedObjectName());

			OutOfBandPackager::pack (wd, position, t);
		}
	}

	JavaString result(t);
	return result.getReturnValue();
}

//----------------------------------------------------------------------

jstring JNICALL ScriptMethodsChatNamespace::packOutOfBandProsePackage(JNIEnv * env, jobject self, jstring target, jint position, jobject stringId, jlong objActor, jlong objTarget, jlong objOther, jobject nameIdActor, jobject nameIdTarget, jobject nameIdOther, jstring nameActor, jstring nameTarget, jstring nameOther, jint digitInteger, jfloat digitFloat, jboolean complexGrammar)
{
	UNREF(env);
	UNREF(self);

	if (!stringId)
	{
		DEBUG_WARNING (true, ("JavaLibrary::packOutOfBandProsePackage attempt to pack prose package with nullptr stringId"));
		return 0;
	}

	Unicode::String t;

	if (target)
	{
		JavaStringParam jt(target);
		if(! JavaLibrary::convert(jt, t))
		{
			DEBUG_WARNING (true, ("JavaLibrary::packOutOfBandProsePackage unable to convert target string"));
			return 0;
		}
	}

	ProsePackage pp (complexGrammar);

	if (!ScriptConversion::convert (stringId, pp.stringId))
	{
		DEBUG_WARNING (true, ("JavaLibrary::packOutOfBandProsePackage unable to convert StringId"));
		return 0;
	}

	pp.actor.id = NetworkId(objActor);
	if (objActor && pp.actor.id == NetworkId::cms_invalid)
	{
		DEBUG_WARNING (true, ("JavaLibrary::packOutOfBandProsePackage unable to convert actor id"));
		return 0;
	}

	pp.target.id = NetworkId(objTarget);
	if (objTarget && pp.target.id == NetworkId::cms_invalid)
	{
		DEBUG_WARNING (true, ("JavaLibrary::packOutOfBandProsePackage unable to convert target id"));
		return 0;
	}

	pp.other.id = NetworkId(objOther);
	if (objOther && pp.other.id == NetworkId::cms_invalid)
	{
		DEBUG_WARNING (true, ("JavaLibrary::packOutOfBandProsePackage unable to convert other id"));
		return 0;
	}

	if (nameIdActor)
	{
		if (!ScriptConversion::convert(nameIdActor, pp.actor.stringId))
		{
			DEBUG_WARNING (true, ("JavaLibrary::packOutOfBandProsePackage unable to convert actor id"));
			return 0;
		}
	}
	else if (nameActor)
	{
		const JavaStringParam jt(nameActor);
		if(! JavaLibrary::convert(jt, pp.actor.str))
		{
			DEBUG_WARNING (true, ("JavaLibrary::packOutOfBandProsePackage unable to convert actor name"));
			return 0;
		}
	}

	if (nameIdTarget)
	{
		if (!ScriptConversion::convert(nameIdTarget, pp.target.stringId))
		{
			DEBUG_WARNING (true, ("JavaLibrary::packOutOfBandProsePackage unable to convert target id"));
			return 0;
		}
	}
	else if (nameTarget)
	{
		const JavaStringParam jt(nameTarget);
		if(! JavaLibrary::convert(jt, pp.target.str))
		{
			DEBUG_WARNING (true, ("JavaLibrary::packOutOfBandProsePackage unable to convert target name"));
			return 0;
		}
	}

	if (nameIdOther)
	{
		if (!ScriptConversion::convert(nameIdOther, pp.other.stringId))
		{
			DEBUG_WARNING (true, ("JavaLibrary::packOutOfBandProsePackage unable to convert other id"));
			return 0;
		}
	}
	else if (nameOther)
	{
		const JavaStringParam jt(nameOther);
		if(! JavaLibrary::convert(jt, pp.other.str))
		{
			DEBUG_WARNING (true, ("JavaLibrary::packOutOfBandProsePackage unable to convert other name"));
			return 0;
		}
	}

	pp.digitInteger = digitInteger;
	pp.digitFloat   = digitFloat;

	Archive::ByteStream bs;
	Archive::put(bs, pp);

	OutOfBandPackager::pack(bs, OutOfBandPackager::OT_prosePackage, position, t);

	JavaString result(t);
	return result.getReturnValue();
}


//-----------------------------------------------------------------------

void JNICALL ScriptMethodsChatNamespace::chatSendToRoom(JNIEnv * env, jobject self, jstring from, jstring roomName, jstring message, jstring outOfBand)
{
	JavaStringParam jspf(from);
	std::string f;
	if(! JavaLibrary::convert(jspf, f))
		return;

	JavaStringParam jsprn(roomName);
	std::string r;
	if(! JavaLibrary::convert(jsprn, r))
		return;

	Unicode::String m;
	if (message)
	{
		JavaStringParam jspm(message);
		if(! JavaLibrary::convert(jspm, m))
			return;
	}

	Unicode::String o;
	if (outOfBand)
	{
		JavaStringParam jspo(outOfBand);
		if(! JavaLibrary::convert(jspo, o))
			return;
	}

	if (o.empty () && m.empty ())
	{
		DEBUG_WARNING (true, ("chatSendToRoom with empty message _and_ oob"));
		return;
	}

	Chat::sendToRoom(f, r, m, o);
}

//-----------------------------------------------------------------------

void JNICALL ScriptMethodsChatNamespace::chatRemoveModeratorFromRoom(JNIEnv * env, jobject self, jstring roomName, jstring moderatorName)
{
	JavaStringParam jsprn(roomName);
	std::string r;
	if(! JavaLibrary::convert(jsprn, r))
		return;

	JavaStringParam jspmn(moderatorName);
	std::string m;
	if(! JavaLibrary::convert(jspmn, m))
		return;

	Chat::removeModeratorFromRoom(m, r);
}

//-----------------------------------------------------------------------

void JNICALL ScriptMethodsChatNamespace::chatSendInstantMessage(JNIEnv * env, jobject self, jstring from, jstring to, jstring message, jstring outOfBand)
{
	JavaStringParam jspf(from);
	std::string f;
	if(! JavaLibrary::convert(jspf, f))
		return;

	JavaStringParam jspt(to);
	std::string t;
	if(! JavaLibrary::convert(jspt, t))
		return;

	Unicode::String m;
	if (message)
	{
		JavaStringParam jspm(message);
		if(! JavaLibrary::convert(jspm, m))
			return;
	}

	Unicode::String o;
	if (outOfBand)
	{
		JavaStringParam jspo(outOfBand);
		if(! JavaLibrary::convert(jspo, o))
			return;
	}

	if (o.empty () && m.empty ())
	{
		DEBUG_WARNING (true, ("chatSendInstantMessage with empty message _and_ oob"));
		return;
	}

	Chat::sendInstantMessage(f, t, m, o);
}

//-----------------------------------------------------------------------

void JNICALL ScriptMethodsChatNamespace::chatSendPersistentMessage(JNIEnv * env, jobject self, jstring from, jstring to, jstring message, jstring outOfBand, jstring subject)
{
	JavaStringParam jspf(from);
	std::string f;
	if(! JavaLibrary::convert(jspf, f))
		return;

	JavaStringParam jspt(to);
	std::string t;
	if(! JavaLibrary::convert(jspt, t))
		return;

	Unicode::String m;
	if (message)
	{
		JavaStringParam jspm(message);
		if(! JavaLibrary::convert(jspm, m))
			return;
	}

	Unicode::String o;
	if (outOfBand)
	{
		JavaStringParam jspo(outOfBand);
		if(! JavaLibrary::convert(jspo, o))
			return;
	}

	Unicode::String s;
	if (subject)
	{
		JavaStringParam jsps(subject);
		if(! JavaLibrary::convert(jsps, s))
			return;
	}

	Chat::sendPersistentMessage(f, t, s, m, o);
}

//----------------------------------------------------------------------

void JNICALL ScriptMethodsChatNamespace::chatSendSystemMessage(JNIEnv * env, jobject self, jstring _to, jstring _message, jstring _oob)
{
	UNREF(self);

	if (!_to)
		return;

	Unicode::String to;
	{
		const JavaStringParam jt(_to);
		if(!JavaLibrary::convert(jt, to))
			return;
	}

	Unicode::String message;

	if (_message)
	{
		const JavaStringParam jt(_message);
		if (!JavaLibrary::convert(jt, message))
			return;
	}

	Unicode::String oob;

	if (_oob)
	{
		const JavaStringParam jt(_oob);
		if (!JavaLibrary::convert(jt, oob))
			return;
	}

	if (message.empty () && oob.empty ())
	{
		DEBUG_WARNING (true, ("chatSendSystemMessage with empty message _and_ oob"));
	}

	Chat::sendSystemMessage(Unicode::wideToNarrow (to), message, oob);
}

//----------------------------------------------------------------------

void JNICALL ScriptMethodsChatNamespace::chatSendSystemMessageObjId(JNIEnv * env, jobject self, jlong _to, jstring _message, jstring _oob)
{
	UNREF(self);

	if (!_to)
		return;


	const ServerObject * player       = 0;

	if(!JavaLibrary::getObject(_to, player))
	{
		DEBUG_WARNING (true, ("ScriptMethodsChat JavaLibrary::chatSendSystemMessageObjId failed bad source object (not nullptr)"));
		if (ConfigServerScript::allowDebugConsoleMessages())
		{
			fprintf(stderr, "WARNING: JavaLibrary::chatSendSystemMessageObjId: failed bad source object (not nullptr)\n");
			JavaLibrary::printJavaStack();
		}
		return;
	}

	Unicode::String message;

	if (_message)
	{
		const JavaStringParam jt(_message);
		if (!JavaLibrary::convert(jt, message))
			return;
	}

	Unicode::String oob;

	if (_oob)
	{
		const JavaStringParam jt(_oob);
		if (!JavaLibrary::convert(jt, oob))
			return;
	}

	if (message.empty () && oob.empty ())
	{
		DEBUG_WARNING (true, ("chatSendSystemMessage with empty message _and_ oob"));
	}

	Chat::sendSystemMessage(*player, message, oob);
}

//----------------------------------------------------------------------

jstring JNICALL ScriptMethodsChatNamespace::getEmoteFromCrc(JNIEnv * env, jobject self, jint crc)
{
	std::string emoteName;
	if (!SocialsManager::getSocialNameByCrc(crc, emoteName))
		return 0;

	return JavaString(emoteName).getReturnValue();
}

//----------------------------------------------------------------------

void JNICALL ScriptMethodsChatNamespace::chatSendQuestSystemMessage(JNIEnv * /*env*/, jobject /*self*/, jlong jobject_networkId, jstring jstring_message, jstring jstring_oob)
{
	ServerObject const * serverObject = 0;
	{
		if (!JavaLibrary::getObject(jobject_networkId, serverObject))
		{
			//@TODO: this function should not be used with object ids of players that are not on the server, however right
			// now we have no other method.  when we have a method that reliably sends these messages across server, use it and put this back in

			//JAVA_THROW_SCRIPT_EXCEPTION(true, ("WARNING: ScriptMethodsChat JavaLibrary::chatSendQuestSystemMessage: could not convert parameter 1 to a valid networkId\n"));
			//printJavaStack();
			return;
		}
	}

	Unicode::String message;
	{
		JavaStringParam const jt(jstring_message);
		IGNORE_RETURN(JavaLibrary::convert(jt, message));
	}

	Unicode::String oob;
	{
		JavaStringParam const jt(jstring_oob);
		IGNORE_RETURN(!JavaLibrary::convert(jt, oob));
	}

	JAVA_THROW_SCRIPT_EXCEPTION(message.empty() && oob.empty(), ("ScriptMethodsChat JavaLibrary::chatSendQuestSystemMessage: all parameters were valid, but empty"));

	Chat::sendQuestSystemMessage(*serverObject, message, oob);
}

//-----------------------------------------------------------------------

void JNICALL ScriptMethodsChatNamespace::chatSendFactionalSystemMessagePlanet(JNIEnv * env, jobject self, jstring prosePackage, jobject loc, jfloat radius, jboolean notifyImperial, jboolean notifyRebel)
{
	if (!prosePackage)
		return;

	JavaStringParam jsProsePackage(prosePackage);
	Unicode::String strProsePackage;
	if(! JavaLibrary::convert(jsProsePackage, strProsePackage))
		return;

	Vector vectorLocation;
	if (loc)
	{
		if (!ScriptConversion::convertWorld(loc, vectorLocation))
			return;
	}

	if (!notifyImperial && !notifyRebel)
		return;

	// send to planet server for distribution to all game servers
	// running this planet; each game server will then check for
	// all players within the range and send the player the message;
	// we need to do this because the range could cover multiple game servers
	FactionalSystemMessage m(strProsePackage, vectorLocation, (loc ? radius : -1.0f), notifyImperial, notifyRebel);
	GameServer::getInstance().sendToPlanetServer(m);
}

//----------------------------------------------------------------------
