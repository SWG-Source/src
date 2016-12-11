//========================================================================
//
// MessageQueueCombatAction.h
//
// copyright 2001 Sony Online Entertainment, Inc.
// All rights reserved.
//
//========================================================================


#ifndef INCLUDED_MessageQueueCombatAction_H
#define INCLUDED_MessageQueueCombatAction_H

#include "sharedFoundation/MessageQueue.h"
#include "sharedFoundation/NetworkId.h"
#include "sharedMath/Vector.h"
#include "swgSharedUtility/CombatEngineData.h"
#include "swgSharedUtility/Postures.def"

#include <string>
#include "sharedNetworkMessages/ControllerMessageMacros.h"

class MemoryBlockManager;
class Object;


class MessageQueueCombatAction : public MessageQueue::Data
{
	CONTROLLER_MESSAGE_INTERFACE;

public:
	struct AttackerData
	{
		AttackerData();

		NetworkId            id;
		NetworkId            weapon;		 // attacker's weapon; if cms_invalid, use current weapon
		Postures::Enumerator endPosture;	 // attacker's posture at the end of the frame
		int                  trailBits;      // bitfield for trail special effects.
		int                  clientEffectId; // id for client effect to play on the attacker.
		int                  actionNameCrc;  // crc for action used by attacker
		bool                 useLocation;    // if true, there is a location as the target/defender
		Vector               targetLocation; // valid if useLocation == TRUE, target of attack
		NetworkId            targetCell;     // valid if useLocation == TRUE
	};
	
	struct DefenderData
	{
		DefenderData();

		NetworkId                       id;
		Postures::Enumerator            endPosture;     // defender's posture at the end of the frame
		CombatEngineData::CombatDefense defense;
		int                             clientEffectId; // id for client effect to play on the defender.
		int                             hitLocation;    // location to target and play hit
		int                             damageAmount;   // amount of damage being applied
	};

	typedef std::vector<DefenderData>  DefenderDataVector;

	typedef void (*ActionNameLookupFunction)(uint32 actionId, char *buffer, int bufferLength);

public:

	static void setActionNameLookupFunction(ActionNameLookupFunction actionNameLookupFunction);

public:

	         MessageQueueCombatAction(void);
	virtual ~MessageQueueCombatAction();

	MessageQueueCombatAction&	operator=	(const MessageQueueCombatAction & source);
	MessageQueueCombatAction(const MessageQueueCombatAction & source);

	uint32 getActionId(void) const;
	void setActionId(uint32 id);
	void setActionId(const std::string & id);
	const AttackerData & getAttacker(void) const;
	void setAttacker(const NetworkId & attacker, const NetworkId & weapon);
	void setAttacker(const NetworkId & attacker, const NetworkId & weapon, Postures::Enumerator posture);
	void setAttackerPosture(Postures::Enumerator posture);
	void setAttackerTrailBits(int trailBits);
	void setAttackerClientEffectId(int clientEffectId);
	void setAttackerActionNameCrc(int actionNameCrc);
	void setAttackerTargetLocation(const Vector &location, const NetworkId &cell, bool useLocation);
	const DefenderDataVector &getDefenders(void) const;
	void addDefender(const NetworkId & defender, CombatEngineData::CombatDefense defense);
	void addDefender(const NetworkId & defender, CombatEngineData::CombatDefense defense, Postures::Enumerator posture, int clientEffectId, int hitLocation, int damageAmount);
	void setDefenderPosture(const NetworkId & defender, Postures::Enumerator posture);
	void setDefenderPosture(const NetworkId & defender, Postures::Enumerator posture, int indexHint);
	void setDefenderDamageAmount(const NetworkId & defender, int damageAmount);
	
	void debugDump() const;

	// If this value matches between two packets, they can be merged
	uint32 getComparisonChecksum( void ) const;
	void   mergePacket( MessageQueueCombatAction* pOther );

private:

	uint32              m_actionId;
	AttackerData        m_attacker;
	DefenderDataVector  m_defenders;
};


inline uint32 MessageQueueCombatAction::getActionId(void) const
{
	return m_actionId;
}	// MessageQueueCombatAction::getActionId

inline void MessageQueueCombatAction::setActionId(uint32 id)
{
	m_actionId = id;
}	// MessageQueueCombatAction::setActionId

inline const MessageQueueCombatAction::AttackerData & 
	MessageQueueCombatAction::getAttacker(void) const
{
	return m_attacker;
}	// MessageQueueCombatAction::getAttacker

inline const std::vector<MessageQueueCombatAction::DefenderData> & 
	MessageQueueCombatAction::getDefenders(void) const
{
	return m_defenders;
}	// MessageQueueCombatAction::getDefenders


#endif	// INCLUDED_MessageQueueCombatAction_H
