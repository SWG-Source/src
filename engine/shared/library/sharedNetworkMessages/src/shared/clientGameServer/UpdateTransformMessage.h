// ======================================================================
//
// UpdateTransformMessage.h
// copyright 2003 Sony Online Entertainment
//
// ======================================================================

#ifndef	INCLUDED_UpdateTransformMessage_H
#define	INCLUDED_UpdateTransformMessage_H

// ======================================================================

#include "sharedNetworkMessages/GameNetworkMessage.h"

// ----------------------------------------------------------------------

class Transform;

//Optimzed packet to update a remote object on a client from a server

class UpdateTransformMessage : public GameNetworkMessage
{
public:
	UpdateTransformMessage	(const NetworkId& newNetworkId, int32 sequenceNumber, const Transform& transform, int8 speed, float lookAtYaw, bool useLookAtYaw);
	explicit UpdateTransformMessage	(Archive::ReadIterator & source);
	~UpdateTransformMessage	();

	const NetworkId&    getNetworkId() const;
	int                 getSequenceNumber() const;
	int8                getSpeed() const;
	const Transform     getTransform() const;
	float				getLookAtYaw() const;
	bool				getUseLookAtYaw() const;

	static void install();
	static void remove();
	static void* operator new (size_t size);
	static void  operator delete (void* pointer);

	

private:

	Archive::AutoVariable<NetworkId>   m_networkId;
	Archive::AutoVariable<int16>       m_positionX;
	Archive::AutoVariable<int16>       m_positionY;
	Archive::AutoVariable<int16>       m_positionZ;
	Archive::AutoVariable<int32>       m_sequenceNumber;
	Archive::AutoVariable<int8>        m_speed;
	Archive::AutoVariable<int8>        m_yaw;
	Archive::AutoVariable<int8>        m_lookAtYaw;
	Archive::AutoVariable<int8>        m_useLookAtYaw;
	
	//todo speed

};

// ======================================================================

inline const NetworkId& UpdateTransformMessage::getNetworkId() const
{
	return m_networkId.get();
}

inline int UpdateTransformMessage::getSequenceNumber() const
{
	return m_sequenceNumber.get();
}

inline int8 UpdateTransformMessage::getSpeed() const
{
	return m_speed.get();
}

inline float UpdateTransformMessage::getLookAtYaw() const
{
	return getUseLookAtYaw() ? (static_cast<float>(m_lookAtYaw.get()) / 16) : 0.f;
}

inline bool UpdateTransformMessage::getUseLookAtYaw() const
{
	return static_cast<bool>(m_useLookAtYaw.get());
}

// ======================================================================


#endif

