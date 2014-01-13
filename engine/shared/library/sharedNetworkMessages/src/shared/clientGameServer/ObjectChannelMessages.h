// ObjectChannelMessages.h
// copyright 2000 Verant Interactive
// Author: Justin Randall

#ifndef	_OBJECT_CHANNEL_MESSAGES_H
#define	_OBJECT_CHANNEL_MESSAGES_H

//-----------------------------------------------------------------------

#include "sharedFoundation/MessageQueue.h"
#include "sharedNetworkMessages/GameNetworkMessage.h"

class Object;

//-----------------------------------------------------------------------


class ObjControllerMessage : public GameNetworkMessage
{
public:
	ObjControllerMessage	(const NetworkId newNetworkId, 
							const uint32 newMessage, 
							const float newValue, 
							const uint32 newFlags,
							MessageQueue::Data* newData);
	explicit ObjControllerMessage	(Archive::ReadIterator & source);
	virtual	~ObjControllerMessage	();
	virtual void          pack(Archive::ByteStream & target) const;
	MessageQueue::Data* getData(void);
	const MessageQueue::Data* getData(void) const;
	const uint32          getFlags(void) const;
	const int32           getMessage(void) const;
	const NetworkId       getNetworkId(void) const;
	const real            getValue(void) const;

	static void  install (void);
	static void  remove (void);
	static void* operator new (size_t size);
	static void  operator delete (void* pointer);

private:
	mutable MessageQueue::Data*   data;
	Archive::AutoVariable<uint32>            flags;
	Archive::AutoVariable<int32>             message;
	Archive::AutoVariable<NetworkId>               networkId;
	Archive::AutoVariable<real>              value;
	//Todo: add getReliable() method

	ObjControllerMessage();
	ObjControllerMessage(const ObjControllerMessage&);
	ObjControllerMessage& operator= (const ObjControllerMessage&);


};

//-----------------------------------------------------------------------

inline MessageQueue::Data* ObjControllerMessage::getData(void)
{
	return data;
}

//-----------------------------------------------------------------------

inline const MessageQueue::Data* ObjControllerMessage::getData(void) const
{
	return data;
}

//-----------------------------------------------------------------------

inline const uint32 ObjControllerMessage::getFlags(void) const
{
	return flags.get();
}

//-----------------------------------------------------------------------

inline const int32 ObjControllerMessage::getMessage(void) const
{
	return message.get();
}

//-----------------------------------------------------------------------

inline const NetworkId ObjControllerMessage::getNetworkId(void) const
{
	return networkId.get();
}

//-----------------------------------------------------------------------

inline const real ObjControllerMessage::getValue(void) const
{
	return value.get();
}

//-----------------------------------------------------------------------



#endif	// _OBJECT_CHANNEL_MESSAGES_H
