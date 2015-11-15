//======================================================================
//
// ShipDamageMessage.cpp
// copyright (c) 2004 Sony Online Entertainment
//
//======================================================================

#include "sharedNetworkMessages/FirstSharedNetworkMessages.h"
#include "sharedNetworkMessages/ShipDamageMessage.h"

#include "sharedFoundation/NetworkIdArchive.h"
#include "sharedMathArchive/VectorArchive.h"

//======================================================================

ShipDamageMessage::ShipDamageMessage() :
m_attackerNetworkId(),
m_attackerPosition(),
m_damageTotal(0.0f),
m_isLocal(false)
{
}

//----------------------------------------------------------------------

ShipDamageMessage::ShipDamageMessage(NetworkId const & attackerId, Vector const & attackerPos, float damageTotal, bool isLocal) :
m_attackerNetworkId(attackerId),
m_attackerPosition(attackerPos),
m_damageTotal(damageTotal),
m_isLocal(isLocal)
{
}

//----------------------------------------------------------------------

NetworkId const & ShipDamageMessage::getAttackerNetworkId() const
{
	return m_attackerNetworkId;
}

//----------------------------------------------------------------------

void ShipDamageMessage::setAttackerNetworkId(NetworkId const & attackerId)
{
	m_attackerNetworkId = attackerId;
}

//----------------------------------------------------------------------

Vector const & ShipDamageMessage::getAttackerPosition() const
{
	return m_attackerPosition;
}

//----------------------------------------------------------------------

void ShipDamageMessage::setAttackerPosition(Vector const & attackerPos)
{
	m_attackerPosition = attackerPos;
}

//----------------------------------------------------------------------

float ShipDamageMessage::getDamageTotal() const
{
	return m_damageTotal;
}

//----------------------------------------------------------------------

void ShipDamageMessage::setDamageTotal(float damage)
{
	m_damageTotal = damage;
}

//----------------------------------------------------------------------

bool ShipDamageMessage::getIsLocal() const
{
	return m_isLocal;
}

//----------------------------------------------------------------------

void ShipDamageMessage::setIsLocal(bool isLocal)
{
	m_isLocal = isLocal;
}

//----------------------------------------------------------------------

void ShipDamageMessage::get(Archive::ReadIterator & source)
{
	Archive::get(source, m_attackerNetworkId);
	Archive::get(source, m_attackerPosition);
	Archive::get(source, m_damageTotal);
}


//----------------------------------------------------------------------

void ShipDamageMessage::put(Archive::ByteStream & target) const
{
	Archive::put(target, m_attackerNetworkId);
	Archive::put(target, m_attackerPosition);
	Archive::put(target, m_damageTotal);
}


//======================================================================
