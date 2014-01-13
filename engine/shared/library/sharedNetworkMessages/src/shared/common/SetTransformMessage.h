// SetTransformMessage.h
// copyright 2002 Sony Online Entertainment


#ifndef	_SetTransformMessage_H
#define	_SetTransformMessage_H

//-----------------------------------------------------------------------

class Object;

#include "Archive/AutoDeltaByteStream.h"
#include "sharedNetworkMessages/GameNetworkMessage.h"
#include "sharedMath/Transform.h"
#include "sharedMathArchive/TransformArchive.h"

//-----------------------------------------------------------------------

class SetTransformMessage : public GameNetworkMessage
{
public: //ctor/dtor
	SetTransformMessage   (const NetworkId & id, const Transform& transform, const NetworkId& cellId);
	explicit SetTransformMessage   (Archive::ReadIterator & source);
	~SetTransformMessage  ();

public: // methods
	const NetworkId &            getNetworkId       () const;
	const Transform &            getTransform       () const;
	const NetworkId &            getCellId          () const;

	void                         pack               (Archive::ByteStream & target) const;

private: 
	Archive::AutoVariable<NetworkId>            m_id;
	Archive::AutoVariable<Transform>           m_transform;
	Archive::AutoVariable<NetworkId>            m_cellId;
};

//-----------------------------------------------------------------------
const NetworkId & SetTransformMessage::getNetworkId() const
{
	return m_id.get();
}
//-----------------------------------------------------------------------
const Transform & SetTransformMessage::getTransform() const
{
	return m_transform.get();
}
//-----------------------------------------------------------------------
const NetworkId & SetTransformMessage::getCellId() const
{
	return m_cellId.get();
}
//-----------------------------------------------------------------------


#endif // _SetTransformMessage_H
