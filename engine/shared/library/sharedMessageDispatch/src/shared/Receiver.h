#ifndef	INCLUDED_Receiver_H
#define	INCLUDED_Receiver_H

//---------------------------------------------------------------------
#pragma warning ( disable : 4786 ) // symbol truncated to 255 characters in debug info

namespace MessageDispatch
{

//---------------------------------------------------------------------

class MessageBase;
class Emitter;

//---------------------------------------------------------------------
/**
	@brief Abstract base class describing an object that can receive
	messages emitted by Emitter objects.

	@author Justin Randall
*/
class Receiver
{
public:
	Receiver();
	virtual ~Receiver();

	void connectToEmitter(const Emitter & target, const char * const messageTypeName);
	void connectToEmitter(const Emitter & target, const MessageBase & source);
	void connectToMessage(const char * const messageTypeName);
	void connectToMessage(const MessageBase & source);
	void disconnectFromEmitter(const Emitter & target, const char * const messageType);
	void disconnectFromEmitter(const Emitter & target, const MessageBase & source);
	void disconnectFromEmitter(const Emitter & target, const unsigned long int messageType);
	void disconnectFromMessage(const char * const messageTypeName);
	void disconnectFromMessage(const MessageBase & source);
	void disconnectFromMessage(const unsigned long int messageType);
	void disconnectAll();
	void emitterDestroyed(Emitter & target);
	void setHasTargets(bool targets);
	bool getHasTargets() const;

	/** Pure virtual */
	virtual void receiveMessage(const Emitter & source, const MessageBase & message) = 0;
private:

	struct EmitterTargets;	
	EmitterTargets *    emitterTargets;
	bool                hasTargets;
};

// ----------------------------------------------------------------------

class GlobalFunctionReceiver : public Receiver
{
public:

	typedef void (*Function)(const Emitter &, const MessageBase &);
	
public:

	GlobalFunctionReceiver(Function function);
	virtual ~GlobalFunctionReceiver();
	virtual void receiveMessage(const Emitter &source, const MessageBase &message);

private:

	Function m_function;
};

// ----------------------------------------------------------------------

template <typename T>
class MemberFunctionReceiver : public Receiver
{
public:

	typedef void (T::*TMemberFunction)(const Emitter &, const MessageBase &);

public:

	MemberFunctionReceiver(T &object, TMemberFunction memberFunction);
	virtual ~MemberFunctionReceiver();
	virtual void receiveMessage(const Emitter &source, const MessageBase &message);

private:
	T                *m_object;
	TMemberFunction   m_memberFunction;
};

// ----------------------------------------------------------------------

template <typename T>
MemberFunctionReceiver<T>::MemberFunctionReceiver(T &object, TMemberFunction memberFunction)
: Receiver(),
	m_object(&object),
	m_memberFunction(memberFunction)
{
}

template <typename T>
MemberFunctionReceiver<T>::~MemberFunctionReceiver()
{
}

template <typename T>
void MemberFunctionReceiver<T>::receiveMessage(const Emitter &source, const MessageBase &message)
{
	(m_object->*m_memberFunction)(source, message);
}

//---------------------------------------------------------------------

}//namespace MessageDispatch

#endif	// _Receiver_H
