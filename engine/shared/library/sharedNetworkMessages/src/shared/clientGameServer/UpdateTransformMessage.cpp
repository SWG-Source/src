// UpdateTransformMessage.cpp
// copyright 2003 Sony Online Entertainment


//-----------------------------------------------------------------------

#include "sharedNetworkMessages/FirstSharedNetworkMessages.h"
#include "sharedNetworkMessages/UpdateTransformMessage.h"

#include "sharedFoundation/MemoryBlockManager.h"

#include "sharedMath/Transform.h"
#include "sharedMath/Vector.h"

//-----------------------------------------------------------------------
namespace UpdateTransformMessageNamespace
{
	MemoryBlockManager* ms_memoryBlockManager;
}

using namespace UpdateTransformMessageNamespace;

//-------------------------------------------------------------------

void UpdateTransformMessage::install (void)
{
	DEBUG_FATAL (ms_memoryBlockManager, ("UpdateTransformMessage already installed"));

	ms_memoryBlockManager = new MemoryBlockManager ("UpdateTransformMessage::ms_memoryBlockManager", true, sizeof(UpdateTransformMessage), 0, 0, 0);
}

//-------------------------------------------------------------------

void UpdateTransformMessage::remove (void)
{
	DEBUG_FATAL(!ms_memoryBlockManager, ("UpdateTransformMessage is not installed"));

	delete ms_memoryBlockManager;
	ms_memoryBlockManager = 0;
}

//-------------------------------------------------------------------

void *UpdateTransformMessage::operator new (size_t size)
{
	UNREF (size);
	NOT_NULL (ms_memoryBlockManager);
	DEBUG_FATAL (size != sizeof (UpdateTransformMessage), ("bad size"));
	DEBUG_FATAL (size != static_cast<size_t> (ms_memoryBlockManager->getElementSize()), ("UpdateTransformMessage installed with bad size"));

	return ms_memoryBlockManager->allocate ();
}

//-------------------------------------------------------------------

void UpdateTransformMessage::operator delete (void *pointer)
{
	NOT_NULL (ms_memoryBlockManager);

	ms_memoryBlockManager->free (pointer);
}

//-------------------------------------------------------------------

UpdateTransformMessage::UpdateTransformMessage(const NetworkId& newNetworkId, int32 sequenceNumber, const Transform& transform, int8 speed, float lookAtYaw, bool useLookAtYaw) :
GameNetworkMessage("UpdateTransformMessage"),
m_networkId(newNetworkId),
m_positionX(0),
m_positionY(0),
m_positionZ(0),
m_sequenceNumber(sequenceNumber),
m_speed(speed),
m_yaw(0),
m_lookAtYaw(0),
m_useLookAtYaw(static_cast<uint8>(useLookAtYaw))
{
	addVariable(m_networkId);
	addVariable(m_positionX);
	addVariable(m_positionY);
	addVariable(m_positionZ);
	addVariable(m_sequenceNumber);
	addVariable(m_speed);
	addVariable(m_yaw);
	addVariable(m_lookAtYaw);
	addVariable(m_useLookAtYaw);

	Vector pos = transform.getPosition_p();
	//Pack position as fixed point integers
	m_positionX.set(static_cast<int16>(pos.x * 4));
	m_positionY.set(static_cast<int16>(pos.y * 4));
	m_positionZ.set(static_cast<int16>(pos.z * 4));

	m_yaw.set(static_cast<int8>(transform.getLocalFrameK_p().theta() * 16) );

	m_lookAtYaw.set(static_cast<int8>(lookAtYaw * 16));	
}

//-----------------------------------------------------------------------

UpdateTransformMessage::UpdateTransformMessage(Archive::ReadIterator & source) :
GameNetworkMessage("UpdateTransformMessage"),
m_networkId(NetworkId::cms_invalid),
m_positionX(0),
m_positionY(0),
m_positionZ(0),
m_sequenceNumber(0),
m_speed(0),
m_yaw(0),
m_lookAtYaw(0),
m_useLookAtYaw(0)
{
	addVariable(m_networkId);
	addVariable(m_positionX);
	addVariable(m_positionY);
	addVariable(m_positionZ);
	addVariable(m_sequenceNumber);
	addVariable(m_speed);
	addVariable(m_yaw);
	addVariable(m_lookAtYaw);
	addVariable(m_useLookAtYaw);
	AutoByteStream::unpack(source);
}

//-----------------------------------------------------------------------

UpdateTransformMessage::~UpdateTransformMessage()
{
}

//-----------------------------------------------------------------------

const Transform UpdateTransformMessage::getTransform() const
{
	Transform transform;
	Vector pos;
	pos.x = static_cast<float>(m_positionX.get()) / 4;
	pos.y = static_cast<float>(m_positionY.get()) / 4;
	pos.z = static_cast<float>(m_positionZ.get()) / 4;

	transform.setPosition_p(pos);
	transform.yaw_l( static_cast<float>(m_yaw.get()) / 16 );
	transform.reorthonormalize();
	return transform;

}

//-----------------------------------------------------------------------

