// MissionObject.cpp
// Copyright 2000-01, Sony Online Entertainment Inc., all rights reserved. 
// Author: Justin Randall

//-----------------------------------------------------------------------

#include "serverGame/FirstServerGame.h"
#include "serverGame/MissionObject.h"

#include "UnicodeUtils.h"
#include "boost/smart_ptr.hpp"
#include "serverGame/ContainerInterface.h"
#include "serverGame/GameServer.h"
#include "serverGame/MessageToQueue.h"
#include "serverGame/ObjectIdManager.h"
#include "serverGame/ObjectTracker.h"
#include "serverGame/PlayerCreatureController.h"
#include "serverGame/ServerMissionObjectTemplate.h"
#include "serverGame/ServerObjectTemplate.h"
#include "serverGame/ServerWorld.h"
#include "serverScript/GameScriptObject.h"
#include "serverScript/ScriptDictionary.h"
#include "serverScript/ScriptParameters.h"
#include "sharedFoundation/ConstCharCrcLowerString.h"
#include "sharedFoundation/ConstCharCrcString.h"
#include "sharedGame/WaypointData.h"
#include "sharedMessageDispatch/Transceiver.h"
#include "sharedNetworkMessages/MessageQueueMissionCreateRequestData.h"
#include "sharedNetworkMessages/MessageQueueMissionDetailsResponseData.h"
#include "sharedObject/NetworkIdManager.h"
#include "sharedObject/ObjectTemplateList.h"

//-----------------------------------------------------------------------

const SharedObjectTemplate * MissionObject::m_defaultSharedTemplate = nullptr;
const char * const s_missionObjectTemplateName = "object/mission/base_mission_object.iff";

//-----------------------------------------------------------------------

MissionObject::MissionObject(const ServerMissionObjectTemplate * t) :
IntangibleObject(t),
m_callback(new MessageDispatch::Callback),
m_description(),
m_difficulty(0),
m_endLocation(),
m_missionCreator(),
m_missionType(),
m_reward(0),
m_rootScriptName(),
m_startLocation(),
m_targetAppearance(),
m_title(),
m_missionHolderId(),
m_status(0),
m_targetName()
{
	addMembersToPackages();
	ObjectTracker::addMissionObject();
	Waypoint & w = const_cast<Waypoint &>(m_waypoint.get());
	m_callback->connect(w.getChangeNotification(), *this, &MissionObject::setWaypoint);
	m_callback->connect(w.getLoadNotification(), *this, &MissionObject::onWaypointLoad);
}

//-----------------------------------------------------------------------

MissionObject::~MissionObject()
{
	ObjectTracker::removeMissionObject();
	delete m_callback;
}

//-----------------------------------------------------------------------

MissionObject * MissionObject::createMissionObjectInCreatureMissionBag(CreatureObject * creature)
{
	MissionObject * result = 0;
	ServerObject * missionBag = creature->getMissionBag();
	if(missionBag)
		result = safe_cast<MissionObject *>(ServerWorld::createNewObject(s_missionObjectTemplateName, *missionBag, false));
	return result;										   
}

//-----------------------------------------------------------------------

bool MissionObject::canTrade() const
{
	return false;
}

//-----------------------------------------------------------------------

bool MissionObject::onContainerAboutToLoseItem(ServerObject * destination, ServerObject& item, ServerObject* transferer)
{
	if (transferer)
		return false;
	return ServerObject::onContainerAboutToLoseItem(destination, item, transferer);
}

//-----------------------------------------------------------------------

bool MissionObject::onContainerAboutToTransfer(ServerObject * destination, ServerObject* transferer)
{

	if (transferer)
		return false;
	return ServerObject::onContainerAboutToTransfer(destination, transferer);

}
//-----------------------------------------------------------------------

int MissionObject::onContainerAboutToGainItem(ServerObject& item, ServerObject* transferer)
{
	if (transferer)
		return Container::CEC_BlockedByDestinationContainer;
	return ServerObject::onContainerAboutToGainItem(item, transferer);
}

//-----------------------------------------------------------------------
/**
 * Returns a shared template if none was given for this object.
 *
 * @return the shared template
 */
const SharedObjectTemplate * MissionObject::getDefaultSharedTemplate(void) const
{
static const ConstCharCrcLowerString templateName("object/mission/base/shared_mission_default.iff");

	if (m_defaultSharedTemplate == nullptr)
	{
		m_defaultSharedTemplate = safe_cast<const SharedObjectTemplate *>(
			ObjectTemplateList::fetch(templateName));
		WARNING_STRICT_FATAL(m_defaultSharedTemplate == nullptr, ("Cannot create "
			"default shared object template %s", templateName.getString()));
		if (m_defaultSharedTemplate != nullptr)
			ExitChain::add (removeDefaultTemplate, "MissionObject::removeDefaultTemplate");
	}
	return m_defaultSharedTemplate;
}	// MissionObject::getDefaultSharedTemplate

//-----------------------------------------------------------------------

/**
 * Cleans up the default shared template.
 */
void MissionObject::removeDefaultTemplate(void)
{
	if (m_defaultSharedTemplate != nullptr)
	{
		m_defaultSharedTemplate->releaseReference();
		m_defaultSharedTemplate = nullptr;
	}
}	// MissionObject::removeDefaultTemplate

//-----------------------------------------------------------------------

void MissionObject::abortMission()
{
	ScriptParams params;
	ScriptDictionaryPtr dictionary;
	getScriptObject()->makeScriptDictionary(params, dictionary);
	if (dictionary.get() != nullptr)
	{
		dictionary->serialize();
		MessageToQueue::getInstance().sendMessageToJava(getNetworkId(), 
			"abortMission", dictionary->getSerializedData(), 0, true);
	}
/*
	CreatureObject *player = safe_cast<CreatureObject *>(ServerWorld::findObjectByNetworkId(getMissionHolderId()));
	if (player)
	{
		player->abortMission(this);
	}
	*/
}

//-----------------------------------------------------------------------

void MissionObject::endBaselines()
{
	CreatureObject * c = dynamic_cast<CreatureObject *>(NetworkIdManager::getObjectById(m_missionHolderId.get()));
	if(c)
	{
		setMissionHolderId(c->getNetworkId());
	}
	ServerObject::endBaselines();
}

//-----------------------------------------------------------------------

void MissionObject::endMission(const bool)
{
}

//-----------------------------------------------------------------------

void MissionObject::setMissionHolderId(const NetworkId & h)
{
	m_missionHolderId.set(h);
}

//-----------------------------------------------------------------------

void MissionObject::getAttributes(std::vector<std::pair<std::string, Unicode::String> > &data) const
{
	IntangibleObject::getAttributes(data);
	std::string const & missionType = getMissionTypeString();
	if (strncmp(missionType.c_str(),"space_",6)==0)
	{
		if (missionType=="space_battle")
			data.push_back(std::make_pair(std::string("type"),Unicode::narrowToWide("@space/quest:space_battle"))); // unfortunately, this one is inconsistent
		else if (missionType=="space_mining_destroy")
			data.push_back(std::make_pair(std::string("type"),Unicode::narrowToWide("@space/quest:space_mining_destroy"))); // unfortunately, this one is also inconsistent
		else
			data.push_back(std::make_pair(std::string("type"),Unicode::narrowToWide(std::string("@space/quest:") + std::string(missionType,6))));
	}
	data.push_back(std::make_pair(std::string("description"),Unicode::narrowToWide(std::string("@") + getDescription().getCanonicalRepresentation())));
}

//-----------------------------------------------------------------------

MissionObject * MissionObject::asMissionObject()
{
	return this;
}

//-----------------------------------------------------------------------

MissionObject const * MissionObject::asMissionObject() const
{
	return this;
}

//-----------------------------------------------------------------------

const NetworkId & MissionObject::getMissionHolderId() const
{
	return m_missionHolderId.get();
}

//-----------------------------------------------------------------------

const StringId & MissionObject::getDescription() const
{
	return m_description.get();
}

//-----------------------------------------------------------------------

const int MissionObject::getDifficulty() const
{
	return m_difficulty.get();
}

//-----------------------------------------------------------------------

const Location & MissionObject::getEndLocation() const
{
	return m_endLocation.get();
}

//-----------------------------------------------------------------------

const Unicode::String & MissionObject::getMissionCreator() const
{
	return m_missionCreator.get();
}

//-----------------------------------------------------------------------

const unsigned int MissionObject::getMissionType() const
{
	return m_missionType.get();
}

//-----------------------------------------------------------------------

const std::string & MissionObject::getMissionTypeString() const
{
	// @todo this should be a crc string table!
	static const unsigned int destroy = CrcLowerString::calculateCrc("destroy");
	static const unsigned int recon = CrcLowerString::calculateCrc("recon");
	static const unsigned int deliver = CrcLowerString::calculateCrc("deliver");
	static const unsigned int escorttocreator = CrcLowerString::calculateCrc("escorttocreator");
	static const unsigned int escort = CrcLowerString::calculateCrc("escort");
	static const unsigned int bounty = CrcLowerString::calculateCrc("bounty");
	static const unsigned int survey = CrcLowerString::calculateCrc("survey");
	static const unsigned int crafting = CrcLowerString::calculateCrc("crafting");
	static const unsigned int musician = CrcLowerString::calculateCrc("musician");
	static const unsigned int dancer =  CrcLowerString::calculateCrc("dancer");
	static const unsigned int hunting =  CrcLowerString::calculateCrc("hunting");
	static const unsigned int space_assassination = CrcLowerString::calculateCrc("space_assassination");
	static const unsigned int space_delivery = CrcLowerString::calculateCrc("space_delivery");
	static const unsigned int space_delivery_duty = CrcLowerString::calculateCrc("space_delivery_duty");
	static const unsigned int space_destroy = CrcLowerString::calculateCrc("space_destroy");
	static const unsigned int space_destroy_duty = CrcLowerString::calculateCrc("space_destroy_duty");
	static const unsigned int space_surprise_attack = CrcLowerString::calculateCrc("space_surprise_attack");
	static const unsigned int space_escort = CrcLowerString::calculateCrc("space_escort");
	static const unsigned int space_escort_duty = CrcLowerString::calculateCrc("space_escort duty");
	static const unsigned int space_inspection = CrcLowerString::calculateCrc("space_inspection");
	static const unsigned int space_patrol = CrcLowerString::calculateCrc("space_patrol");
	static const unsigned int space_recovery = CrcLowerString::calculateCrc("space_recovery");
	static const unsigned int space_recovery_duty = CrcLowerString::calculateCrc("space_recovery_duty");
	static const unsigned int space_rescue = CrcLowerString::calculateCrc("space_rescue");
	static const unsigned int space_rescue_duty = CrcLowerString::calculateCrc("space_rescue_duty");
	static const unsigned int space_battle = CrcLowerString::calculateCrc("space_battle");
	static const unsigned int space_survival = CrcLowerString::calculateCrc("space_survival");
	static const unsigned int space_mining_destroy = CrcLowerString::calculateCrc("space_mining_destroy");
	
	if(m_missionType.get() == destroy)
	{
		static const std::string r = "destroy";
		return r;
	}
	else if(m_missionType.get() == recon)
	{
		static const std::string r = "recon";
		return r;
	}
	else if(m_missionType.get() == deliver)
	{
		static const std::string r = "deliver";
		return r;
	}
	else if(m_missionType.get() == escorttocreator)
	{
		static const std::string r = "escorttocreator";
		return r;
	}
	else if(m_missionType.get() == escort)
	{
		static const std::string r = "escort";
		return r;
	}
	else if(m_missionType.get() == bounty)
	{
		static const std::string r = "bounty";
		return r;
	}
	else if(m_missionType.get() == survey)
	{
		static const std::string r = "survey";
		return r;
	}
	else if(m_missionType.get() == crafting)
	{
		static const std::string r = "crafting";
		return r;
	}
	else if(m_missionType.get() == musician)
	{
		static const std::string r = "musician";
		return r;
	}
	else if(m_missionType.get() == dancer)
	{
		static const std::string r = "dancer";
		return r;
	}
	else if(m_missionType.get() == hunting)
	{
		static const std::string r = "hunting";
		return r;
	}
	else if(m_missionType.get() == space_assassination)
	{
		static const std::string r = "space_assassination";
		return r;
	}
	else if(m_missionType.get() == space_delivery)
	{
		static const std::string r = "space_delivery";
		return r;
	}
	else if(m_missionType.get() == space_delivery_duty)
	{
		static const std::string r = "space_delivery_duty";
		return r;
	}
	else if(m_missionType.get() == space_destroy)
	{
		static const std::string r = "space_destroy";
		return r;
	}
	else if(m_missionType.get() == space_destroy_duty)
	{
		static const std::string r = "space_destroy_duty";
		return r;
	}
	else if(m_missionType.get() == space_surprise_attack)
	{
		static const std::string r = "space_surprise_attack";
		return r;
	}
	else if(m_missionType.get() == space_escort)
	{
		static const std::string r = "space_escort";
		return r;
	}
	else if(m_missionType.get() == space_escort_duty)
	{
		static const std::string r = "space_escort_duty";
		return r;
	}
	else if(m_missionType.get() == space_inspection)
	{
		static const std::string r = "space_inspection";
		return r;
	}
	else if(m_missionType.get() == space_patrol)
	{
		static const std::string r = "space_patrol";
		return r;
	}
	else if(m_missionType.get() == space_recovery)
	{
		static const std::string r = "space_recovery";
		return r;
	}
	else if(m_missionType.get() == space_recovery_duty)
	{
		static const std::string r = "space_recovery_duty";
		return r;
	}
	else if(m_missionType.get() == space_rescue)
	{
		static const std::string r = "space_rescue";
		return r;
	}
	else if(m_missionType.get() == space_rescue_duty)
	{
		static const std::string r = "space_rescue_duty";
		return r;
	}
	else if(m_missionType.get() == space_battle)
	{
		static const std::string r = "space_battle";
		return r;
	}
	else if(m_missionType.get() == space_survival)
	{
		static const std::string r = "space_survival";
		return r;
	}
	else if(m_missionType.get() == space_mining_destroy)
	{
		static const std::string r = "space_mining_destroy";
		return r;
	}

	static const std::string unknown("unknown");
	return unknown;
	
}

//-----------------------------------------------------------------------

const int MissionObject::getReward() const
{
	return m_reward.get();
}

//-----------------------------------------------------------------------

const std::string & MissionObject::getRootScriptName() const
{
	return m_rootScriptName.get();
}

//-----------------------------------------------------------------------

const Location & MissionObject::getStartLocation() const
{
	return m_startLocation.get();
}

//-----------------------------------------------------------------------

const int MissionObject::getStatus() const
{
	return m_status.get();
}

//-----------------------------------------------------------------------

const unsigned int MissionObject::getTargetAppearance() const
{
	return m_targetAppearance.get();
}

//-----------------------------------------------------------------------

const std::string & MissionObject::getTargetName() const
{
	return m_targetName.get();
}

//-----------------------------------------------------------------------

const StringId & MissionObject::getTitle() const
{
	return m_title.get();
}

//-----------------------------------------------------------------------

const Waypoint & MissionObject::getWaypoint() const
{
	return m_waypoint.get();
}

//-----------------------------------------------------------------------

void MissionObject::grantMissionTo(const NetworkId & target)
{
	ScriptParams p;
	p.addParam(target);
	getScriptObject()->trigAllScripts(Scripting::TRIG_GRANT_MISSION, p);
}

//-----------------------------------------------------------------------

void MissionObject::onAddedToWorld()
{
	IntangibleObject::onAddedToWorld();
}

//-----------------------------------------------------------------------

void MissionObject::setDescription(const StringId & d)
{
	m_description.set(d);
}

//-----------------------------------------------------------------------

void MissionObject::setDifficulty(const int v)
{
	m_difficulty.set(v);
}

//-----------------------------------------------------------------------

void MissionObject::setEndLocation(const Location & v)
{
	m_endLocation.set(v);
}

//-----------------------------------------------------------------------

void MissionObject::setMissionCreator(const Unicode::String & c)
{
	m_missionCreator.set(c);
}

//-----------------------------------------------------------------------

void MissionObject::setMissionType(const unsigned char t)
{
	m_missionType.set(t);
}

//-----------------------------------------------------------------------

void MissionObject::setMissionType(const std::string & t)
{
	m_missionType.set(CrcLowerString::calculateCrc(t.c_str()));
}

//-----------------------------------------------------------------------

void MissionObject::setReward(const int v)
{
	m_reward.set(v);
}

//-----------------------------------------------------------------------

void MissionObject::setRootScriptName(const std::string & r)
{
	if(! m_rootScriptName.get().empty())
	{
		getScriptObject()->detachScript(getRootScriptName());
	}
	m_rootScriptName.set(r);
	getScriptObject()->attachScript(m_rootScriptName.get(), true);
}

//-----------------------------------------------------------------------

void MissionObject::setStartLocation(const Location & v)
{
	m_startLocation.set(v);
}

//-----------------------------------------------------------------------

void MissionObject::setStatus(int newStatus)
{
	if(newStatus != 0)
	{
		m_status.set(m_status.get() + 1);
	}
}

//-----------------------------------------------------------------------

void MissionObject::setTargetAppearance(const std::string & a)
{
	const ServerObjectTemplate * t = dynamic_cast<const ServerObjectTemplate *>(ObjectTemplateList::fetch(a));
	if(t)
	{
		std::string s = t->getSharedTemplate();
		ConstCharCrcString c(s.c_str());
		setTargetAppearance(c.getCrc());
	}
}

//-----------------------------------------------------------------------

void MissionObject::setTargetAppearance(unsigned int a)
{
	m_targetAppearance.set(a);
}

//-----------------------------------------------------------------------

void MissionObject::setTargetName(const std::string & n)
{
	m_targetName.set(n);
}

//-----------------------------------------------------------------------

void MissionObject::setTitle(const StringId & t)
{
	m_title.set(t);
}

//-----------------------------------------------------------------------

void MissionObject::setWaypoint(const Waypoint & w)
{
	m_waypoint.set(w);
}

//-----------------------------------------------------------------------

void MissionObject::onWaypointLoad(Waypoint::ChangeNotification & c)
{
	Waypoint & w = const_cast<Waypoint &>(m_waypoint.get());
	m_callback->disconnect(w.getChangeNotification(), *this, &MissionObject::setWaypoint);
	m_callback->connect(c, *this, &MissionObject::setWaypoint);
}

// ----------------------------------------------------------------------

void MissionObject::initializeFirstTimeObject()
{
	m_waypoint.set(Waypoint(ObjectIdManager::getNewObjectId()));

	IntangibleObject::initializeFirstTimeObject();
}

// ----------------------------------------------------------------------

void MissionObject::getByteStreamFromAutoVariable(const std::string & name, Archive::ByteStream & target) const
{
	if (name == "moCtsPackUnpack")
	{
		// description
		m_description.pack(target);

		// difficulty
		m_difficulty.pack(target);

		// end location
		m_endLocation.pack(target);

		// mission creator
		m_missionCreator.pack(target);

		// mission type
		m_missionType.pack(target);

		// reward
		m_reward.pack(target);

		// root script name
		m_rootScriptName.pack(target);

		// start location
		m_startLocation.pack(target);

		// target appearance
		m_targetAppearance.pack(target);

		// title
		m_title.pack(target);

		// mission holder id
		// don't need to pack this, will be assigned the id of the destination character object

		// status
		m_status.pack(target);

		// target name
		m_targetName.pack(target);

		// waypoint
		Archive::put(target, m_waypoint.get().getWaypointDataBase());

		// mission location target
		CreatureObject const * containingPlayer = nullptr;
		ServerObject const * parent = safe_cast<ServerObject const*>(ContainerInterface::getContainedByObject(*this));
		while (parent)
		{
			containingPlayer = parent->asCreatureObject();
			if (containingPlayer && PlayerCreatureController::getPlayerObject(containingPlayer))
				break;

			containingPlayer = nullptr;
			parent = safe_cast<ServerObject const*>(ContainerInterface::getContainedByObject(*parent));
		}

		bool hasMissionLocationTarget = false;
		if (containingPlayer)
		{
			std::vector<LocationData> const & locationData = containingPlayer->getLocationTargets();
			for (std::vector<LocationData>::const_iterator iter = locationData.begin(); iter != locationData.end(); ++iter)
			{
				if (iter->name == Unicode::narrowToWide(getNetworkId().getValueString()))
				{
					Archive::put(target, true);
					Archive::put(target, *iter);
					hasMissionLocationTarget = true;
					break;
				}
			}
		}

		if (!hasMissionLocationTarget)
			Archive::put(target, false);
	}
	else
	{
		IntangibleObject::getByteStreamFromAutoVariable(name, target);
	}
}

// ----------------------------------------------------------------------

void MissionObject::setAutoVariableFromByteStream(const std::string & name, const Archive::ByteStream & source)
{
	Archive::ReadIterator ri(source);
	if (name == "moCtsPackUnpack")
	{
		// description
		m_description.unpackDelta(ri);

		// difficulty
		m_difficulty.unpackDelta(ri);

		// end location
		m_endLocation.unpackDelta(ri);

		// mission creator
		m_missionCreator.unpackDelta(ri);

		// mission type
		m_missionType.unpackDelta(ri);

		// reward
		m_reward.unpackDelta(ri);

		// root script name
		m_rootScriptName.unpackDelta(ri);

		// start location
		m_startLocation.unpackDelta(ri);

		// target appearance
		m_targetAppearance.unpackDelta(ri);

		// title
		m_title.unpackDelta(ri);

		// mission holder id
		CreatureObject * containingPlayer = nullptr;
		ServerObject * parent = safe_cast<ServerObject *>(ContainerInterface::getContainedByObject(*this));
		while (parent)
		{
			containingPlayer = parent->asCreatureObject();
			if (containingPlayer && PlayerCreatureController::getPlayerObject(containingPlayer))
				break;

			containingPlayer = nullptr;
			parent = safe_cast<ServerObject *>(ContainerInterface::getContainedByObject(*parent));
		}

		if (containingPlayer)
			m_missionHolderId.set(containingPlayer->getNetworkId());

		// status
		m_status.unpackDelta(ri);

		// target name
		m_targetName.unpackDelta(ri);

		// waypoint
		WaypointDataBase waypointDB;
		Archive::get(ri, waypointDB);

		Waypoint w = m_waypoint.get();

		w.setAppearanceNameCrc(waypointDB.m_appearanceNameCrc);
		w.setLocation(waypointDB.m_location);
		w.setName(waypointDB.m_name);
		w.setColor(waypointDB.m_color);
		w.setActive(waypointDB.m_active);

		m_waypoint.set(w);

		DynamicVariableList const & objvar = getObjVars();
		if (objvar.hasItem("objTargetWaypoint") && (objvar.getType("objTargetWaypoint") == DynamicVariable::NETWORK_ID))
			IGNORE_RETURN(setObjVarItem("objTargetWaypoint", w.getNetworkId()));

		// mission location target
		bool hasMissionLocationTarget;
		Archive::get(ri, hasMissionLocationTarget);
		if (hasMissionLocationTarget)
		{
			LocationData locationData;
			Archive::get(ri, locationData);
			locationData.name = Unicode::narrowToWide(getNetworkId().getValueString());

			if (containingPlayer)
				containingPlayer->addLocationTarget(locationData);
		}

		// questOwner objvar
		if (objvar.hasItem("questOwner") && (objvar.getType("questOwner") == DynamicVariable::NETWORK_ID) && containingPlayer)
			IGNORE_RETURN(setObjVarItem("questOwner", containingPlayer->getNetworkId()));

		// self objvar
		if (objvar.hasItem("self") && (objvar.getType("self") == DynamicVariable::NETWORK_ID))
			IGNORE_RETURN(setObjVarItem("self", getNetworkId()));
	}
	else
	{
		IntangibleObject::setAutoVariableFromByteStream(name, source);
	}

	addObjectToConcludeList();
}

// ======================================================================
