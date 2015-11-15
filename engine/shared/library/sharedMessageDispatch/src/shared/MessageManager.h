#ifndef	INCLUDED_MessageManager_H
#define	INCLUDED_MessageManager_H

namespace MessageDispatch
{

//---------------------------------------------------------------------

class Emitter;
class MessageBase;
class Receiver;

void connectToMessage(const char * const messageTypeName, void (*)(const Emitter &, const MessageBase &));

//---------------------------------------------------------------------
/**
	@brief A Singleton class responsible for broadcasting and tracking
	Receiver, Emitter and Message objects

	@author Justin Randall
*/
class MessageManager
{
public:
	MessageManager();
	~MessageManager();
	void   addReceiver(Receiver & target, const char * const messageTypeName);
	void   addReceiver(Receiver & target, const MessageBase & source);
	void   addReceiver(Receiver & target, const unsigned long int messageType);
	void   addStaticCallback(void (*)(const Emitter &, const MessageBase &), const unsigned long int messageTypeName);
	void   emitMessage(const Emitter & emitter, const MessageBase & message) const;
	void   receiverDestroyed(const Receiver & target);
	void   removeReceiver(const Receiver & target, const char * const messageTypeName);
	void   removeReceiver(const Receiver & target, const unsigned long int messageType);

	static MessageManager & getInstance ();


private:
	struct Data;
	Data *data;

	static MessageManager ms_instance;
};

//----------------------------------------------------------------------

inline MessageManager & MessageManager::getInstance ()
{
	return ms_instance;
}

//---------------------------------------------------------------------

}//namespace MessageDispatch

#endif	// _MessageManager_H
