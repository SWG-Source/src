//========================================================================
//
// MessageQueueCombatAction.cpp
//
// copyright 2001 Sony Online Entertainment
//
//========================================================================

#include "swgSharedNetworkMessages/FirstSwgSharedNetworkMessages.h"
#include "MessageQueueCombatAction.h"
#include "sharedFoundation/CrcLowerString.h"

#include "swgSharedUtility/Postures.h"

#include <algorithm>
#include <vector>
#include "sharedFoundation/MemoryBlockManager.h"
#include "sharedFoundation/ExitChain.h"
#include "sharedFoundation/GameControllerMessage.h"
#include "sharedMathArchive/VectorArchive.h"
#include "sharedNetworkMessages/ControllerMessageFactory.h"
#include "sharedObject/NetworkIdManager.h"

// ======================================================================

namespace MessageQueueCombatActionNamespace
{
	MessageQueueCombatAction::ActionNameLookupFunction  s_actionNameLookupFunction;
}

using namespace MessageQueueCombatActionNamespace;

//===================================================================

CONTROLLER_MESSAGE_IMPLEMENTATION(MessageQueueCombatAction, CM_combatAction);

// ======================================================================

/**
 * Class constructor.
 */
MessageQueueCombatAction::AttackerData::AttackerData()
:	id(),
	weapon(),
	endPosture(Postures::Upright),
	trailBits(0),
 clientEffectId(0),
 actionNameCrc(0),
 useLocation(false),
 targetLocation()
{
} // MessageQueueCombatAction::AttackerData::AttackerData

/**
 * Class constructor.
 */
MessageQueueCombatAction::DefenderData::DefenderData() 
:	id(),
	endPosture(Postures::Upright),
	defense(CombatEngineData::CD_miss),
	clientEffectId(0),
	hitLocation(0),
	damageAmount(0)
{
} // MessageQueueCombatAction::DefenderData:::DefenderData

// ======================================================================

void MessageQueueCombatAction::setActionNameLookupFunction(ActionNameLookupFunction actionNameLookupFunction)
{
	s_actionNameLookupFunction = actionNameLookupFunction;
}

// ======================================================================

/**
 * Class constructor.
 */
MessageQueueCombatAction::MessageQueueCombatAction(void)
:	Data(),
	m_actionId(0),
	m_attacker(),
	m_defenders()
{
}	// MessageQueueCombatAction::MessageQueueCombatAction

/**
 * Class destructor.
 */
MessageQueueCombatAction::~MessageQueueCombatAction()
{
}	// MessageQueueCombatAction::~MessageQueueCombatAction

/**
 * Sets the action id based on a string value.
 *
 * @param id		the action id
 */
void MessageQueueCombatAction::setActionId(const std::string & id)
{
	m_actionId = CrcLowerString::calculateCrc(id.c_str());
}	// MessageQueueCombatAction::setActionId

/**
 * Sets the info for the attacker of this action.
 *
 * @param attacker				the attacker's id
 */
void MessageQueueCombatAction::setAttacker(const NetworkId & attacker, 
	const NetworkId & weapon)
{
	m_attacker.id = attacker;
	m_attacker.weapon = weapon;
}	// MessageQueueCombatAction::setAttacker(const NetworkId &)

/**
 * Sets the info for the attacker of this action.
 *
 * @param attacker		the attacker's id
 * @param posture		the attacker's final posture
 */
void MessageQueueCombatAction::setAttacker(const NetworkId & attacker, 
	const NetworkId & weapon, Postures::Enumerator posture)
{
	m_attacker.id = attacker;
	m_attacker.weapon = weapon;
	m_attacker.endPosture = posture;
}	// MessageQueueCombatAction::setAttacker(const NetworkId &, Postures::Enumerator)

/**
 * Sets the posture for the attacker.
 *
 * @param posture		the attacker's final posture
 */
void MessageQueueCombatAction::setAttackerPosture(Postures::Enumerator posture)
{
	m_attacker.endPosture = posture;
}	// MessageQueueCombatAction::setAttackerPosture

/**
 * Sets the trail bit field (trail special effect flags) for the attacker.
 *
 * @param trailBits  the bit field describing the enabled SFX trails for the attacker.
 */
void MessageQueueCombatAction::setAttackerTrailBits(int trailBits)
{
	m_attacker.trailBits = trailBits;
}	// MessageQueueCombatAction::setAttackerTrailBits

/**
 * Sets the client effect id used for the deferred client effect played on the attacker.
 *
 * @param clientEffectId   the id (row number) of the effect to play on the client after a
 *                         deferred amount of time.  A value of zero (the default) will play no effect.
 */
void MessageQueueCombatAction::setAttackerActionNameCrc(int actionNameCrc)
{
	m_attacker.actionNameCrc = actionNameCrc;
}	// MessageQueueCombatAction::setAttackerActionNameCrc

/**
 * Sets the client effect id used for the deferred client effect played on the attacker.
 *
 * @param clientEffectId   the id (row number) of the effect to play on the client after a
 *                         deferred amount of time.  A value of zero (the default) will play no effect.
 */
void MessageQueueCombatAction::setAttackerClientEffectId(int clientEffectId)
{
	m_attacker.clientEffectId = clientEffectId;
}	// MessageQueueCombatAction::setAttackerClientEffectId

/**
 * Sets the location for a location-based attack.
 *
 * @param location   world-space location that is the target of the attack, if useLocation is true
 * @param useLocation whether the attack targets a location
 */
void MessageQueueCombatAction::setAttackerTargetLocation(const Vector &location, const NetworkId &targetCell, bool useLocation)
{
	m_attacker.useLocation = useLocation;
	m_attacker.targetLocation = location;
	m_attacker.targetCell = targetCell;
}

/**
 * Adds info about a defender of this action.
 *
 * @param defender		the defender's id
 * @param defense		the defense used on a failed attack
 */
void MessageQueueCombatAction::addDefender(const NetworkId & defender, 
	CombatEngineData::CombatDefense defense)
{
	m_defenders.push_back(DefenderData());
	DefenderData & data = m_defenders.back();
	data.id = defender;
	data.defense = defense;
}	// MessageQueueCombatAction::addDefender(const NetworkId &, CombatEngineData::CombatDefense)

/**
 * Adds info about a defender of this action.
 *
 * @param defender		the defender's id
 * @param defense		the defense used on a failed attack
 * @param posture		the defender's final posture
 * @param clientEffectId        the deferred client effect id for the defender.
 * @param damageAmount  the amount of damage applied to the defender
 */
void MessageQueueCombatAction::addDefender(const NetworkId & defender, 
	CombatEngineData::CombatDefense defense, Postures::Enumerator posture,
	int clientEffectId, int hitLocation, int damageAmount)
{
	m_defenders.push_back(DefenderData());
	DefenderData & data = m_defenders.back();
	data.id = defender;
	data.defense = defense;
	data.endPosture = posture;
	data.clientEffectId = clientEffectId;
	data.hitLocation = hitLocation;
	data.damageAmount = damageAmount;
}	// MessageQueueCombatAction::addDefender(const NetworkId &, CombatEngineData::CombatDefense, Postures::Enumerator)

/**
 * Sets the final posture for a defender.
 *
 * @param defender		the defender's id
 * @param posture		the defender's posture
 */
void MessageQueueCombatAction::setDefenderPosture(const NetworkId & defender, Postures::Enumerator posture)
{
	for (DefenderDataVector::iterator iter(m_defenders.begin()); 
		iter != m_defenders.end(); ++iter)
	{
		DefenderData & data = *iter;
		if (data.id == defender)
		{
			data.endPosture = posture;
			return;
		}
	}
}	// MessageQueueCombatAction::setDefenderPosture(const NetworkId &, Postures::Enumerator)

/**
 * Sets the final posture for a defender.
 *
 * @param defender		the defender's id
 * @param posture		the defender's posture
 * @param indexHint		hint as to the vector index of the defender
 */
void MessageQueueCombatAction::setDefenderPosture(const NetworkId & defender, Postures::Enumerator posture, int indexHint)
{
	if (indexHint >= 0 && indexHint < static_cast<int>(m_defenders.size()))
	{
		DefenderData & data = m_defenders[static_cast<DefenderDataVector::size_type>(indexHint)];
		if (data.id == defender)
		{
			data.endPosture = posture;
			return;
		}
	}
	setDefenderPosture(defender, posture);
}	// MessageQueueCombatAction::setDefenderPosture(const NetworkId &, Postures::Enumerator, int)


/**
 * Sets the amount of damge appied by the attacker.
 *
 * @param damage amount 
 */
void MessageQueueCombatAction::setDefenderDamageAmount(const NetworkId & defender, int damageAmount)
{
	for (DefenderDataVector::iterator iter(m_defenders.begin()); 
		iter != m_defenders.end(); ++iter)
	{
		DefenderData & data = *iter;
		if (data.id == defender)
		{
			data.damageAmount = damageAmount;
			return;
		}
	}
}	// MessageQueueCombatAction::setDamageAmount

/**
 * Dump out the contents of the message via DEBUG_REPORT_LOG.
 */
void MessageQueueCombatAction::debugDump() const
{
	//-- Get attacker and defender objects.
	const Object *const attackerObject = NetworkIdManager::getObjectById(m_attacker.id);
	UNREF(attackerObject);

	const Object *const weaponObject   = NetworkIdManager::getObjectById(m_attacker.weapon);
	UNREF(weaponObject);

	//-- Lookup action name if we have it.
	bool  hasActionName;
	char  actionName[256];

	if (s_actionNameLookupFunction != nullptr)
	{
		hasActionName = true;
		(*s_actionNameLookupFunction)(m_actionId, actionName, sizeof(actionName));
	}
	else
		hasActionName = false;

	//-- Start the dump.
	DEBUG_REPORT_LOG(true, ("MQCA: START.\n"));

	// Print action name or id.
	if (!hasActionName)
		DEBUG_REPORT_LOG(true, ("MQCA: actionId [0x%08X].\n", static_cast<unsigned int>(m_actionId)));
	else
		DEBUG_REPORT_LOG(true, ("MQCA: action name [%s].\n", actionName));

	// Print attacker info.
	DEBUG_REPORT_LOG(true, ("MQCA: attacker: id              =[%s].\n", m_attacker.id.getValueString().c_str()));
	DEBUG_REPORT_LOG(true, ("MQCA: attacker: template        =[%s].\n", attackerObject ? attackerObject->getObjectTemplateName() : "<nullptr object>"));
	DEBUG_REPORT_LOG(true, ("MQCA: attacker: weapon          =[%s].\n", m_attacker.weapon.getValueString().c_str()));
	DEBUG_REPORT_LOG(true, ("MQCA: attacker: weapon template =[%s].\n", weaponObject ? weaponObject->getObjectTemplateName() : "<nullptr weapon>"));
	DEBUG_REPORT_LOG(true, ("MQCA: attacker: end posture     =[%s].\n", Postures::getPostureName(m_attacker.endPosture)));
	DEBUG_REPORT_LOG(true, ("MQCA: attacker: trailBits       =[0x%02x].\n", m_attacker.trailBits));
	DEBUG_REPORT_LOG(true, ("MQCA: attacker: client effect id=[%d].\n", m_attacker.clientEffectId));
	DEBUG_REPORT_LOG(true, ("MQCA: attacker: action name crc =[%d].\n", m_attacker.actionNameCrc));
	DEBUG_REPORT_LOG(true, ("MQCA: attacker: use location    =[%s].\n", m_attacker.useLocation ? "TRUE" : "FALSE"));
	if(m_attacker.useLocation)
	{
		DEBUG_REPORT_LOG(true, ("MQCA: attacker: location target =[%5.0f %5.0f %5.0f]", m_attacker.targetLocation.x, m_attacker.targetLocation.y, m_attacker.targetLocation.z));
		DEBUG_REPORT_LOG(true, ("MQCA: attacker: location target cell =[%s]", m_attacker.targetCell.getValueString().c_str()));
	}

	// Print per-defender info.
	const DefenderDataVector::size_type defenderCount = m_defenders.size();
	DEBUG_REPORT_LOG(true, ("MQCA: defender count [%u].\n", defenderCount));

	for (DefenderDataVector::size_type i = 0; i < defenderCount; ++i)
	{
		const DefenderData &data = m_defenders[i];
		const Object *const defenderObject = NetworkIdManager::getObjectById(data.id);
		UNREF(defenderObject);

		DEBUG_REPORT_LOG(true, ("MQCA: defender #%d: id              =[%s].\n", i + 1, data.id.getValueString().c_str()));
		DEBUG_REPORT_LOG(true, ("MQCA: defender #%d: template        =[%s].\n", i + 1, defenderObject ? defenderObject->getObjectTemplateName() : "<nullptr object>"));
		DEBUG_REPORT_LOG(true, ("MQCA: defender #%d: end posture     =[%s].\n", i + 1, Postures::getPostureName(data.endPosture)));
		DEBUG_REPORT_LOG(true, ("MQCA: defender #%d: defense         =[%s].\n", i + 1, CombatEngineData::getCombatDefenseName(data.defense)));
		DEBUG_REPORT_LOG(true, ("MQCA: defender #%d: client effect id=[%d].\n", i + 1, data.clientEffectId));
	}

	DEBUG_REPORT_LOG(true, ("MQCA: END.\n"));
} // MessageQueueCombatAction::debugDump()



//
// Get a checksum value that can be used to determine if two MessageQueueCombatAction packets
//   can be combined into one.  We only look at the attacker info because there can be multiple
//   defenders.
//
uint32 MessageQueueCombatAction::getComparisonChecksum( void ) const
{
	uint32 retval = 0;
	retval += m_actionId;
	retval += NetworkId::Hash()(m_attacker.id);
	retval += NetworkId::Hash()(m_attacker.weapon);
	retval += int(m_attacker.endPosture);
	retval += m_attacker.trailBits;
	retval += m_attacker.clientEffectId;
	retval += m_attacker.actionNameCrc;
	retval += m_attacker.useLocation;
	retval += static_cast<uint32>(m_attacker.targetLocation.x);
	retval += static_cast<uint32>(m_attacker.targetLocation.y);
	retval += static_cast<uint32>(m_attacker.targetLocation.z);

	return retval;
}

//
// Merge packets (possibly adding another defender if need be...)
//
void MessageQueueCombatAction::mergePacket( MessageQueueCombatAction* pOther )
{
	DefenderDataVector::const_iterator oiter;
	const DefenderDataVector& odefenders = pOther->getDefenders();
	
        for ( oiter = odefenders.begin(); oiter != odefenders.end(); ++oiter )
	{
#if 0
/********  This code does damage aggregation.  It may look like a bug since it will make damage numbers higher *********
		bool b_defender_merged = false;
		DefenderDataVector::iterator miter;
		for ( miter = m_defenders.begin(); miter != m_defenders.end(); ++miter )
		{
			if (( miter->id == oiter->id ) && ( miter->clientEffectId == oiter->clientEffectId ))
			{
				miter->damageAmount += oiter->damageAmount;
				b_defender_merged = true;
			}
		}
		if ( ! b_defender_merged )  // Add another defender for the one we couldn't merge
**********************************************************************************************************************/
#endif
		{
			m_defenders.push_back( *oiter );
		}
	}

}



// ----------------------------------------------------------------------

void MessageQueueCombatAction::pack(const MessageQueue::Data* const data, Archive::ByteStream & target)
{
	const MessageQueueCombatAction* const msg = safe_cast<const MessageQueueCombatAction*> (data);
	
	if (msg)
	{
		Archive::put(target, msg->getActionId());
		
		// write the attacker data
		const MessageQueueCombatAction::AttackerData & attacker = msg->getAttacker();
		Archive::put(target, attacker.id);
		Archive::put(target, attacker.weapon);
		Archive::put(target, attacker.endPosture);
		const uint8 trailBits = static_cast<uint8>(attacker.trailBits);
		Archive::put(target, trailBits);
		Archive::put(target, static_cast<uint8>(attacker.clientEffectId));
		Archive::put(target, attacker.actionNameCrc);
		Archive::put(target, attacker.useLocation);
		if(attacker.useLocation)
		{
			Archive::put(target, attacker.targetLocation);
			Archive::put(target, attacker.targetCell);
		}
		
		// write the defender data
		const std::vector<MessageQueueCombatAction::DefenderData> & defenderList = msg->getDefenders();
		uint16 count = static_cast<uint16>(defenderList.size());
		Archive::put(target, count);
		for (std::vector<MessageQueueCombatAction::DefenderData>::const_iterator 
			iter = defenderList.begin(); iter != defenderList.end(); ++iter)
		{
			Archive::put(target, (*iter).id);
			Archive::put(target, (*iter).endPosture);
			uint8 defense = static_cast<uint8>((*iter).defense);
			Archive::put(target, defense);
			Archive::put(target, static_cast<uint8>((*iter).clientEffectId));
			Archive::put(target, static_cast<uint8>((*iter).hitLocation));
			Archive::put(target, static_cast<uint16>((*iter).damageAmount));
		}
	}
}	// ControllerMessageFactory::packCombatAction

//-----------------------------------------------------------------------


MessageQueue::Data* MessageQueueCombatAction::unpack(Archive::ReadIterator & source)
{
	NetworkId id, weapon;
	Postures::Enumerator postureEnd;
	uint32 actionId;
	uint8 defense;
	int actionNameCrc;
	
	MessageQueueCombatAction * msg = new MessageQueueCombatAction;
	
	Archive::get(source, actionId);
	msg->setActionId(actionId);
	
	// read the attacker data
	Archive::get(source, id);
	Archive::get(source, weapon);
	Archive::get(source, postureEnd);
	
	msg->setAttacker(id, weapon, postureEnd);

	uint8 trailBits = 0;
	Archive::get(source, trailBits);
	msg->setAttackerTrailBits(static_cast<int>(trailBits));

	uint8 clientEffectId;
	Archive::get(source, clientEffectId);
	msg->setAttackerClientEffectId(static_cast<int>(clientEffectId));

	Archive::get(source, actionNameCrc);
	msg->setAttackerActionNameCrc(actionNameCrc);

	bool useLocation;
	Archive::get(source, useLocation);
	
	Vector locationTarget;
	NetworkId cellTarget;
	if(useLocation)
	{
		Archive::get(source, locationTarget);
		Archive::get(source, cellTarget);
	}
	msg->setAttackerTargetLocation(locationTarget, cellTarget, useLocation);
     
	// read the defender data
	uint8 hitLocation;
	uint16 count;
	uint16 damageAmount;
	Archive::get(source, count);
	for (uint16 i = 0; i < count; ++i)
	{
		Archive::get(source, id);
		Archive::get(source, postureEnd);
		Archive::get(source, defense);
		Archive::get(source, clientEffectId);
		Archive::get(source, hitLocation);
		Archive::get(source, damageAmount);
		msg->addDefender(id, static_cast<CombatEngineData::CombatDefense>(defense), postureEnd, clientEffectId, hitLocation, damageAmount);
	}
	
	return msg;
}	// ControllerMessageFactory::unpackCombatAction

//-----------------------------------------------------------------------
