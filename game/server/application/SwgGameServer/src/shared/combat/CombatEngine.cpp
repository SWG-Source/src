//========================================================================
//
// CombatEngine.cpp
//
// copyright 2001 Sony Online Entertainment
//
//========================================================================


#include "FirstSwgGameServer.h"
#include "CombatEngine.h"

#include "ConfigCombatEngine.h"
#include "serverGame/Client.h"
#include "serverGame/ConfigServerGame.h" //@todo code reorg, need another cfg file
#include "serverGame/ConsoleManager.h"
#include "serverGame/ContainerInterface.h"
#include "serverGame/CreatureController.h"
#include "serverGame/CreatureObject.h"
#include "serverGame/ServerObject.h"
#include "serverGame/ServerWorld.h"
#include "serverGame/TangibleObject.h"
#include "serverGame/WeaponObject.h"
#include "serverScript/GameScriptObject.h"
#include "serverScript/ScriptFunctionTable.h"
#include "serverScript/ScriptParameters.h"
#include "sharedFoundation/ConfigFile.h"
#include "sharedFoundation/ConstCharCrcLowerString.h"
#include "sharedFoundation/GameControllerMessage.h"
#include "sharedFoundation/GameControllerMessage.h"
#include "sharedFoundation/Misc.h"
#include "sharedGame/CommandTable.h"
#include "sharedGame/GameObjectTypes.h"
#include "sharedGame/SharedObjectTemplate.h"
#include "sharedNetworkMessages/MessageQueueGenericValueType.h"
#include "sharedNetworkMessages/MessageQueueNetworkId.h"
#include "sharedTerrain/TerrainObject.h"
#include "swgServerNetworkMessages/MessageQueueCombatDamageList.h"
#include "swgServerNetworkMessages/MessageQueueDirectDamage.h"
#include "swgSharedNetworkMessages/CombatActionCompleteMessage.h"
#include "swgSharedNetworkMessages/MessageQueueCombatAction.h"
#include "swgSharedNetworkMessages/MessageQueueCombatDamage.h"

#include <cstdio>

static const std::string UNSKILLED_COMMAND("unskilled");
static const ConstCharCrcLowerString COMBAT_TARGET_COMMAND("combattarget");
static const ConstCharCrcLowerString AIM_COMMAND("aim");
static const ConstCharCrcLowerString DEFAULT_ATTACK_COMMAND("defaultattack");


//------------------------------------------------------------------------
// static class variables

uint16 CombatEngine::ms_nextActionId = 0;


//------------------------------------------------------------------------
// class functions

/**
 * Initializes the combat engine.
 */
void CombatEngine::install(void)
{
	ConfigCombatEngine::install();

	// set up command queue combat functions
	CommandTable::addCppFunction("aim", aim);

	ExitChain::add(CombatEngine::remove, "CombatEngine::remove");
}	// CombatEngine::install

/**
 * Cleans up the combat engine.
 */
void CombatEngine::remove(void)
{
	ConfigCombatEngine::remove();
}	// CombatEngine::remove

/**
 * Reloads the combat data values from the combat config file.
 *
 * @return true on success, false on fail
 */
bool CombatEngine::reloadCombatData(void)
{
	// @todo: can we synchronize this between servers?
	ConfigCombatEngine::remove();
	ConfigCombatEngine::install();
	return true;
}	// CombatEngine::reloadCombatData

/**
 * Adds an aim to an attacker's next attack.
 *
 * @param actor			the attacker
 */
void CombatEngine::aim(const Command &, const NetworkId & actor, const NetworkId &, const Unicode::String &)
{
	CachedNetworkId attackerId(actor);
	TangibleObject * attacker = dynamic_cast<TangibleObject *>(attackerId.getObject());

	if (attacker != nullptr)
	{
		attacker->addAim();
	}
}	// CombatEngine::aim

/**
 * Adds a target action to the end of an object's action queue.
 *
 * @param attacker		the object attacking
 * @param targets		who is being targeted
 *
 * @return true on success, false on fail
 */
bool CombatEngine::addTargetAction(TangibleObject & attacker, 
	const CombatEngineData::TargetIdList & targets)
{
	// @todo: support tangible attacks
	CreatureObject * const creatureAttacker = attacker.asCreatureObject ();
	if (creatureAttacker == nullptr)
	{
		WARNING_STRICT_FATAL(true, ("Tangible attackers not supported yet"));
		return false;
	}

	creatureAttacker->commandQueueEnqueue(CommandTable::getCommand(
		COMBAT_TARGET_COMMAND.getCrc()), targets[0], Unicode::String());
	return true;
}	// CombatEngine::addTargetAction

/**
 * Adds an attack action to the end of an object's action queue.
 *
 * @param attacker		the object attacking
 * @param weapon		the weapon being used (0 = attacker's primary weapon)
 * @param weaponMode	the mode of the weapon being used (0 = primary, -1 = current mode)
 *
 * @return true on success, false on fail
 */
bool CombatEngine::addAttackAction(TangibleObject & attacker, 
	const NetworkId & weapon, int weaponMode)
{
	// @todo: support tangible attacks
	CreatureObject * const creatureAttacker = attacker.asCreatureObject ();
	if (creatureAttacker == nullptr)
	{
		WARNING_STRICT_FATAL(true, ("Tangible attackers not supported yet"));
		return false;
	}

	// set up the weapon id and mode into a params string
	char buffer[64];
	sprintf(buffer, "%s %d", weapon.getValueString().c_str(), weaponMode);
	Unicode::String params(Unicode::narrowToWide(buffer));
	
	creatureAttacker->commandQueueEnqueue(CommandTable::getCommand(
		DEFAULT_ATTACK_COMMAND.getCrc()), NetworkId::cms_invalid, params);
	return true;
}	// CombatEngine::addAttackAction

/**
 * Adds an aim action to the end of an object's action queue.
 *
 * @param attacker		the object attacking
 * 
 * @return true on success, false on fail
 */
bool CombatEngine::addAimAction(TangibleObject & attacker)
{
	if (!attacker.isInCombat())
		return false;

	// @todo: support tangible attacks
	CreatureObject * const creatureAttacker = attacker.asCreatureObject ();
	if (creatureAttacker == nullptr)
	{
		WARNING_STRICT_FATAL(true, ("Tangible attackers not supported yet"));
		return false;
	}

	creatureAttacker->commandQueueEnqueue(CommandTable::getCommand(
		AIM_COMMAND.getCrc()), NetworkId::cms_invalid, Unicode::String());
	return true;
}	// CombatEngine::addAimAction

/**
 * Determines the specific damage to apply to the defender after a successful 
 * attack.
 *
 * @param attacker			the attacker
 * @param defender			the defender
 * @param weapon			the attacker's weapon
 * @param damageAmount		base damage done
 * @param hitLocation		where the defender was hit
 *
 * @return true on success, false if there was an error
 */
bool CombatEngine::onSuccessfulAttack(const TangibleObject & attacker, 
	TangibleObject & defender, const WeaponObject & weapon, int damageAmount, 
	int hitLocation)
{
	const bool creatureDefender = defender.asCreatureObject() != nullptr;
	const bool isVehicle        = GameObjectTypes::isTypeOf (defender.getGameObjectType (), SharedObjectTemplate::GOT_vehicle);

	// if attacking an object, always hit location 0
	if (!creatureDefender || isVehicle)
		hitLocation = 0;

	// get the damage profile for the hit location
	const ConfigCombatEngine::SkeletonAttackMod & skeletonAttackMod = 
		ConfigCombatEngine::getSkeletonAttackMod(static_cast<ServerTangibleObjectTemplate::CombatSkeleton>(defender.getCombatSkeleton()));
	if (hitLocation < 0 || hitLocation >= static_cast<int>(skeletonAttackMod.attackMods.size()))
		return false;
	const ConfigCombatEngineData::BodyAttackMod & hitLocationData = 
		skeletonAttackMod.attackMods[static_cast<size_t>(hitLocation)];

	// put a attribMod structure on the defender's damage list for each type of 
	// damage received
	DamageList damageList;
	if (creatureDefender && !isVehicle)
	{
		computeCreatureDamage(&hitLocationData, damageAmount, damageList);
	}
	else
	{
		computeObjectDamage(&hitLocationData, damageAmount, damageList);
	}
	if (damageList.empty())
		return false;
	
	// determine if the defender was wounded
	bool isWounded = false;
	
	if (creatureDefender && !isVehicle)
	{
		float woundChance = weapon.getWoundChance() + ConfigCombatEngine::getWoundChance();
		if (hitLocation)
			woundChance += hitLocationData.toWoundBonus;
		if (Random::randomReal(0.0f, 100.0f) <= woundChance)
			isWounded = true;
	}
	
	// apply the damage
	if (defender.isAuthoritative())
	{
		// Ensure the combat data has been initialized
		defender.createCombatData();

		CombatEngineData::DefenseData & defenseData = defender.getCombatData()->defenseData;
		defenseData.damage.push_back(CombatEngineData::DamageData());
		CombatEngineData::DamageData &damageData = defenseData.damage.back();
		damageData.attackerId = attacker.getNetworkId();
		damageData.weaponId = weapon.getNetworkId();
		damageData.damageType = static_cast<CombatEngineData::DamageType>(static_cast<int>(weapon.getDamageType()));
		damageData.hitLocationIndex = static_cast<uint16>(hitLocation);
		damageData.damage.insert(damageData.damage.end(), damageList.begin(), damageList.end());
		damageData.wounded = isWounded;
	}
	else
	{
		TangibleController * const tangibleController = (defender.getController() != nullptr) ? defender.getController()->asTangibleController() : nullptr;

		if (tangibleController == nullptr)
		{
			WARNING_STRICT_FATAL(true, ("CombatEngine::onSuccessfulAttack non-auth "
				"defender %s doesn't have a TangibleController!", 
				defender.getNetworkId().getValueString().c_str()));
			return false;
		}

		// send the damage info to the authoritative object
		MessageQueueCombatDamageList * message = new MessageQueueCombatDamageList();
		CombatEngineData::DamageData & damageData = message->addDamage();

		damageData.attackerId = attacker.getNetworkId();
		damageData.weaponId = weapon.getNetworkId();
		damageData.damageType = static_cast<CombatEngineData::DamageType>(static_cast<int>(weapon.getDamageType()));
		damageData.hitLocationIndex = static_cast<uint16>(hitLocation);
		damageData.damage.insert(damageData.damage.end(), damageList.begin(), damageList.end());
		damageData.wounded = isWounded;

		tangibleController->appendMessage(
			CM_combatDamageList,
			0.0f, 
			message,
			GameControllerMessageFlags::SEND |
			GameControllerMessageFlags::RELIABLE |
			GameControllerMessageFlags::DEST_AUTH_SERVER
			);
	}//lint !e429 // Custodial pointer 'message' has not been freed or returned // the controller queue will manage freeing the memory

	return true;
}	// CombatEngine::onSuccessfulAttack

/**
 * Determines the specific damage to apply to the defender after a successful 
 * attack.
 *
 * @param attacker			the attacker
 * @param defender			the defender
 * @param damageAmount		base damage done
 * @param hitLocation		where the defender was hit
 *
 * @return true on success, false if there was an error
 */
bool CombatEngine::onSuccessfulAttack(const TangibleObject & attacker, 
	TangibleObject & defender, int damageAmount, 
	int hitLocation)
{
	const bool creatureDefender = defender.asCreatureObject() != nullptr;
	const bool isVehicle        = GameObjectTypes::isTypeOf (defender.getGameObjectType (), SharedObjectTemplate::GOT_vehicle);

	// if attacking an object, always hit location 0
	if (!creatureDefender || isVehicle)
		hitLocation = 0;

	// get the damage profile for the hit location
	const ConfigCombatEngine::SkeletonAttackMod & skeletonAttackMod = 
		ConfigCombatEngine::getSkeletonAttackMod(static_cast<ServerTangibleObjectTemplate::CombatSkeleton>(defender.getCombatSkeleton()));
	if (hitLocation < 0 || hitLocation >= static_cast<int>(skeletonAttackMod.attackMods.size()))
		return false;
	const ConfigCombatEngineData::BodyAttackMod & hitLocationData = 
		skeletonAttackMod.attackMods[static_cast<size_t>(hitLocation)];

	// put a attribMod structure on the defender's damage list for each type of 
	// damage received
	DamageList damageList;
	if (creatureDefender && !isVehicle)
	{
		computeCreatureDamage(&hitLocationData, damageAmount, damageList);
	}
	else
	{
		computeObjectDamage(&hitLocationData, damageAmount, damageList);
	}
	if (damageList.empty())
		return false;
	
	// determine if the defender was wounded
	bool isWounded = false;
	
	// apply the damage
	if (defender.isAuthoritative())
	{
		// Ensure the combat data has been initialized
		defender.createCombatData();

		CombatEngineData::DefenseData & defenseData = defender.getCombatData()->defenseData;
		defenseData.damage.push_back(CombatEngineData::DamageData());
		CombatEngineData::DamageData &damageData = defenseData.damage.back();
		damageData.attackerId = attacker.getNetworkId();
		damageData.hitLocationIndex = static_cast<uint16>(hitLocation);
		damageData.damage.insert(damageData.damage.end(), damageList.begin(), damageList.end());
		damageData.wounded = isWounded;
	}
	else
	{
		TangibleController * const tangibleController = (defender.getController() != nullptr) ? defender.getController()->asTangibleController() : nullptr;

		if (tangibleController == nullptr)
		{
			WARNING_STRICT_FATAL(true, ("CombatEngine::onSuccessfulAttack non-auth "
				"defender %s doesn't have a TangibleController!", 
				defender.getNetworkId().getValueString().c_str()));
			return false;
		}

		// send the damage info to the authoritative object
		MessageQueueCombatDamageList * message = new MessageQueueCombatDamageList();
		CombatEngineData::DamageData & damageData = message->addDamage();

		damageData.attackerId = attacker.getNetworkId();
		damageData.hitLocationIndex = static_cast<uint16>(hitLocation);
		damageData.damage.insert(damageData.damage.end(), damageList.begin(), damageList.end());
		damageData.wounded = isWounded;

		tangibleController->appendMessage(
			CM_combatDamageList,
			0.0f, 
			message,
			GameControllerMessageFlags::SEND |
			GameControllerMessageFlags::RELIABLE |
			GameControllerMessageFlags::DEST_AUTH_SERVER
			);
	}//lint !e429 // Custodial pointer 'message' has not been freed or returned // the controller queue will manage freeing the memory

	return true;
}	// CombatEngine::onSuccessfulAttack

/**
 * Damages an object. This damage is immediately applied to the object, outside 
 * the combat loop.
 *
 * @param defender			the defender
 * @param weapon			the weapon the defender is hit by
 * @param damageAmount		base damage done
 * @param hitLocation		where the defender was hit
 *
 * @return true on success, false if there was an error
 */
bool CombatEngine::damage(TangibleObject & defender, const WeaponObject & weapon, 
	int damageAmount, int hitLocation)
{
	if (defender.isAuthoritative())
	{
		CreatureObject * const critter = defender.asCreatureObject();
		const bool isVehicle    = GameObjectTypes::isTypeOf (defender.getGameObjectType (), SharedObjectTemplate::GOT_vehicle);

		// if attacking an object, always hit location 0
		if (critter == nullptr || isVehicle)
			hitLocation = 0;

		// get the damage profile for the hit location
		const ConfigCombatEngine::SkeletonAttackMod & skeletonAttackMod = 
			ConfigCombatEngine::getSkeletonAttackMod(static_cast<ServerTangibleObjectTemplate::CombatSkeleton>(defender.getCombatSkeleton()));
		if (hitLocation < 0 || hitLocation >= static_cast<int>(skeletonAttackMod.attackMods.size()))
			return false;
		const ConfigCombatEngineData::BodyAttackMod & hitLocationData = 
			skeletonAttackMod.attackMods[static_cast<size_t>(hitLocation)];

		// put a attribMod structure on the defender's damage list for each type of 
		// damage received
		DamageList damageList;
		if (critter != nullptr && !isVehicle)
		{
			computeCreatureDamage(&hitLocationData, damageAmount, damageList);
		}
		else
		{
			computeObjectDamage(&hitLocationData, damageAmount, damageList);
		}
		if (damageList.empty())
			return false;

		// determine if the defender was wounded
		bool isWounded = false;
		if (critter && !isVehicle)
		{
			float woundChance = weapon.getWoundChance() + ConfigCombatEngine::getWoundChance();
			if (hitLocation)
				woundChance += hitLocationData.toWoundBonus;
			if (Random::randomReal(0.0f, 100.0f) <= woundChance)
				isWounded = true;
		}

		// apply the damage
		CombatEngineData::DamageData damageData;
		damageData.attackerId = NetworkId::cms_invalid;
		damageData.weaponId = weapon.getNetworkId();
		damageData.damageType = static_cast<CombatEngineData::DamageType>(static_cast<int>(weapon.getDamageType()));
		damageData.hitLocationIndex = static_cast<uint16>(hitLocation);
		damageData.damage.insert(damageData.damage.end(), damageList.begin(), damageList.end());
		damageData.wounded = isWounded;

		if (critter != nullptr || !defender.isDisabled())
		{
			// update the object's hit points or attributes for the damage
			defender.applyDamage(damageData);
		}
	}

	return true;
}	// CombatEngine::damage

/**
 * Damages an object. This damage is immediately applied to the object, outside 
 * the combat loop.
 *
 * @param defender		the object to be damaged
 * @param damageType	the type of damage being done
 * @param hitLocation	the hit location of the damage
 * @param damage		the amount of damage to do
 */
void CombatEngine::damage(TangibleObject & defender, 
	ServerWeaponObjectTemplate::DamageType damageType, uint16 hitLocation, 
	int damageDone)
{
	if (defender.isAuthoritative())
	{
		CreatureObject *critter = defender.asCreatureObject();
		const bool isVehicle    = GameObjectTypes::isTypeOf (defender.getGameObjectType (), SharedObjectTemplate::GOT_vehicle);

		// if attacking an object, always hit location 0
		if (critter == nullptr || isVehicle)
			hitLocation = 0;

		// get the damage profile for the hit location
		const ConfigCombatEngine::SkeletonAttackMod & skeletonAttackMod = 
			ConfigCombatEngine::getSkeletonAttackMod(static_cast<ServerTangibleObjectTemplate::CombatSkeleton>(defender.getCombatSkeleton()));
		if (hitLocation >= static_cast<int>(skeletonAttackMod.attackMods.size()))
			return;
		const ConfigCombatEngineData::BodyAttackMod & hitLocationData = 
			skeletonAttackMod.attackMods[static_cast<size_t>(hitLocation)];

		// get the damage
		DamageList damageList;
		if (critter && !isVehicle)
			CombatEngine::computeCreatureDamage(&hitLocationData, damageDone, damageList);
		else
			CombatEngine::computeObjectDamage(&hitLocationData, damageDone, damageList);

		if (damageList.empty())
			return;

		// apply the damage
		CombatEngineData::DamageData damageData;
		damageData.attackerId = NetworkId::cms_invalid;
		damageData.weaponId = NetworkId::cms_invalid;
		damageData.damageType = static_cast<CombatEngineData::DamageType>(static_cast<int>(damageType));
		damageData.hitLocationIndex = hitLocation;
		damageData.damage.insert(damageData.damage.end(), damageList.begin(), damageList.end());

		if (critter != nullptr || !defender.isDisabled())
		{
			// update the object's hit points or attributes for the damage
			defender.applyDamage(damageData);
		}
	}
	else
	{
		defender.sendControllerMessageToAuthServer(CM_directDamage, 
			new MessageQueueDirectDamage(damageType, hitLocation, damageDone));
	}
}	// CombatEngine::damage

/**
 * Damages all objects within a given area.
 *
 * @param center			the center of the area to damage
 * @param radius			the radius of the area to damage
 * @param damageType		the type of damage being done
 * @param damage			the amount of damage to do
 */
void CombatEngine::damage(const Vector &center, float radius, 
	ServerWeaponObjectTemplate::DamageType damageType, int damageDone)
{
	// determine the damage that will occur to creatures and non-creatures
	DamageList creatureDamageList, objectDamageList;
	computeCreatureDamage(0, damageDone, creatureDamageList);
	computeObjectDamage(0, damageDone, objectDamageList);
	if (creatureDamageList.empty() && objectDamageList.empty())
		return;

	// get everything in the damage area
	std::vector<ServerObject *> targetList;
	ServerWorld::findObjectsInRange(center, radius, targetList);
	if (targetList.empty())
		return;

	std::vector<ServerObject *>::iterator iter;
	for (iter = targetList.begin(); iter != targetList.end(); ++iter)
	{
		ServerObject * const obj = *iter;
		if (!obj)
			continue;

		TangibleObject * const defender = obj->asTangibleObject ();
		if (!defender)
			continue;

		const bool isVehicle    = GameObjectTypes::isTypeOf (defender->getGameObjectType (), SharedObjectTemplate::GOT_vehicle);

		// apply the damage
		{
			// Ensure the combat data has been initialized
			defender->createCombatData();

			CombatEngineData::DefenseData & defenseData = defender->getCombatData()->defenseData;
			defenseData.damage.push_back(CombatEngineData::DamageData());
			CombatEngineData::DamageData &damageData = defenseData.damage.back();
			damageData.attackerId = NetworkId::cms_invalid;
			damageData.weaponId = NetworkId::cms_invalid;
			damageData.damageType = static_cast<CombatEngineData::DamageType>(static_cast<int>(damageType));
			damageData.hitLocationIndex = 0;
		
			if (defender->asCreatureObject () && !isVehicle)
			{
				damageData.damage.insert(damageData.damage.end(), 
										 creatureDamageList.begin(), creatureDamageList.end());
			}
			else
			{
				damageData.damage.insert(damageData.damage.end(), 
										 objectDamageList.begin(), objectDamageList.end());
			}
		}
	}
}	// CombatEngine::damage

/**
 * Determines how a creature will be damaged.
 *
 * @param hitLocation		where on the defender the damage occurred
 * @param damageType		the type of damage being done
 * @param damage			the amount of damage to do
 * @param damageList		list to be filled in with attribMods caused by the damage
 */
void CombatEngine::computeCreatureDamage(
	const ConfigCombatEngineData::BodyAttackMod *hitLocation,
	int damageDone, 
	DamageList & damageList)
{
	// put all damage into health
	damageList.push_back(AttribMod::AttribMod());
	AttribMod::AttribMod & attribMod = damageList.back();
	attribMod.tag = 0;
	attribMod.attrib = Attributes::Health;
	attribMod.value = -damageDone;
	attribMod.attack = 0.0f;
	attribMod.sustain = 0.0f;
	attribMod.decay = ServerWeaponObjectTemplate::AMDS_pool;
	attribMod.flags = AttribMod::AMF_directDamage;
}	// CombatEngine::computeCreatureDamage

/**
 * Determines how a non-creature will be damaged.
 *
 * @param hitLocation		where on the defender the damage occurred
 * @param damageType		the type of damage being done
 * @param damage			the amount of damage to do
 * @param damageList		list to be filled in with attribMods caused by the damage
 */
void CombatEngine::computeObjectDamage(
	const ConfigCombatEngineData::BodyAttackMod *hitLocation,
	int damageDone,
	DamageList & damageList)
{
	int actualDamage = static_cast<int>(floor(damageDone * 1.0f + 0.5f)); //lint !e747  Significant prototype coercion (arg. no. 1) float to double
	if (actualDamage == 0)
		return;

	// create an attribMod structure for damage received
	if (hitLocation && hitLocation->damageBonus[0] != 0)
	{
		if (hitLocation->damageBonus[0] > 0)
		{
			actualDamage += static_cast<int>(floor(actualDamage * hitLocation->
				damageBonus[0] + 0.5f)); //lint !e747  Significant prototype coercion (arg. no. 1) float to double
		}
		else
		{
			actualDamage += static_cast<int>(ceil(actualDamage * hitLocation->
				damageBonus[0] - 0.5f)); //lint !e747  Significant prototype coercion (arg. no. 1) float to double
		}
	}
	damageList.push_back(AttribMod::AttribMod());
	AttribMod::AttribMod & attribMod = damageList.back();
	attribMod.tag = 0;
	attribMod.attrib = ServerObjectTemplate::AT_health;
	attribMod.value = -actualDamage;
	attribMod.attack = 0.0f;
	attribMod.sustain = 0.0f;
	attribMod.decay = ServerWeaponObjectTemplate::AMDS_pool; //lint !e641  Converting enum 'AttribModDecaySpecial' to int
	attribMod.flags = AttribMod::AMF_directDamage;
}	// CombatEngine::computeObjectDamage

/**
 * Applies combat damage done during execute() to an object. Should be called as 
 * part of the object's alter function.
 *
 * @param object		the object to damage
 */
void CombatEngine::alter(TangibleObject & object)
{
	NOT_NULL(object.getController());

	if (   (object.getCombatData() == nullptr)
	    || object.getCombatData()->defenseData.damage.empty())
	{
		return;
	}

	std::vector<CombatEngineData::DamageData> &damageData = object.getCombatData()->defenseData.damage;
	std::vector<CombatEngineData::DamageData>::iterator iter;

	if (object.isAuthoritative())
	{
		// update the object
		// to guard against the list changing while we are iterating over it,
		// we will iterate using index rather than iterator
		for (std::vector<CombatEngineData::DamageData>::size_type i = 0; object.isInCombat() && i < damageData.size(); ++i)
		{
			// since armor == hp now, we need to double check that the object
			// is in combat, since a non-creature could be removed by just
			// damaging their armor
			if (object.isInCombat())
			{
				// update the object's hit points or attributes for the damage
				object.applyDamage(damageData[i]);
			}
		}
	}
	else
	{
		TangibleController * const tangibleController = object.getController()->asTangibleController();

		if (tangibleController == nullptr)
		{
			WARNING_STRICT_FATAL(true, ("CombatEngine::alter non-auth "
				"object %s doesn't have a TangibleController!", 
				object.getNetworkId().getValueString().c_str()));
				return;
		}

		// send the damage info to the authoritative object
		MessageQueueCombatDamageList * message = new MessageQueueCombatDamageList();
		for (iter = damageData.begin(); iter != damageData.end(); ++iter)
			message->addDamage(*iter);
		tangibleController->appendMessage(
			CM_combatDamageList,
			0.0f, 
			message,
			GameControllerMessageFlags::SEND |
			GameControllerMessageFlags::RELIABLE |
			GameControllerMessageFlags::DEST_AUTH_SERVER
			);
	}//lint !e429 Custodial pointer 'message' has not been freed or returned // Controller handles message destruction

	object.clearDamageList();
}	// CombatEngine::alter

