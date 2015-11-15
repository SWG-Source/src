//======================================================================
//
// ShipDamageMessage.h
// copyright (c) 2004 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_ShipDamageMessage_H
#define INCLUDED_ShipDamageMessage_H

#include "sharedMath/Vector.h"
#include "sharedFoundation/NetworkId.h"

//======================================================================

namespace Archive
{
	class ReadIterator;
	class ByteStream;
}

//----------------------------------------------------------------------

class ShipDamageMessage
{
public:
	ShipDamageMessage();
	ShipDamageMessage(NetworkId const & attackerId, Vector const & attackerPos, float damageTotal, bool isLocal = false);

	NetworkId const & getAttackerNetworkId() const;
	void setAttackerNetworkId(NetworkId const & attackerId);

	Vector const & getAttackerPosition() const;
	void setAttackerPosition(Vector const & attackerPos);
	
	float getDamageTotal() const;
	void setDamageTotal(float damage);

	bool getIsLocal() const;
	void setIsLocal(bool isLocal);

	void get(Archive::ReadIterator & source);
	void put(Archive::ByteStream & target) const;

private:
	NetworkId m_attackerNetworkId;
	Vector m_attackerPosition;
	float m_damageTotal;
	bool m_isLocal;
};

//======================================================================

#endif
