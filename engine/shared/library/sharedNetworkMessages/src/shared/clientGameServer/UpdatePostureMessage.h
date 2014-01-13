// UpdatePostureMessage.h
// copyright 2001 Verant Interactive
// 

#ifndef	_UpdatePostureMessage_H
#define	_UpdatePostureMessage_H

//-----------------------------------------------------------------------

class Object;

#include "Archive/AutoDeltaByteStream.h"
#include "sharedNetworkMessages/GameNetworkMessage.h"
#include "sharedFoundation/NetworkId.h"
#include "sharedFoundation/Tag.h"

//-----------------------------------------------------------------------

class UpdatePostureMessage : public GameNetworkMessage
{
public: //ctor/dtor
	UpdatePostureMessage(const NetworkId & sourceObject, uint8 posture);
	explicit UpdatePostureMessage(Archive::ReadIterator & source);

	~UpdatePostureMessage  ();

public: // methods

	uint8            getPosture() const;
	const NetworkId & getTarget() const;

	void  pack(Archive::ByteStream & target) const;
public: // types

private: 
	Archive::AutoVariable<uint8>     m_posture;
	Archive::AutoVariable<NetworkId>  m_target;
};

//-----------------------------------------------------------------------

inline uint8 UpdatePostureMessage::getPosture() const
{
	return m_posture.get();
}

//-----------------------------------------------------------------------

inline const NetworkId& UpdatePostureMessage::getTarget() const
{
	return m_target.get();
}

//-----------------------------------------------------------------------

#endif // _UpdatePostureMessage_H
