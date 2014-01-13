// MessageQueueGenericValueType.h
// Copyright 2000-02, Sony Online Entertainment Inc., all rights reserved. 
// Author: Justin Randall

#ifndef	_INCLUDED_MessageQueueGenericValueType_H
#define	_INCLUDED_MessageQueueGenericValueType_H

//-----------------------------------------------------------------------

#include "sharedFoundation/MessageQueue.h"

//-----------------------------------------------------------------------

template<typename ValueType>
class MessageQueueGenericValueType : public MessageQueue::Data
{
public:
	MessageQueueGenericValueType(const ValueType & value);
	~MessageQueueGenericValueType();

	const ValueType &  getValue  () const;

private:
	MessageQueueGenericValueType & operator = (const MessageQueueGenericValueType & rhs);
	MessageQueueGenericValueType(const MessageQueueGenericValueType & source);

	ValueType  m_value;
};

//-----------------------------------------------------------------------

template<typename ValueType>
inline MessageQueueGenericValueType<ValueType>::MessageQueueGenericValueType(const ValueType & v) :
MessageQueue::Data(),
m_value(v)
{
}

//-----------------------------------------------------------------------

template<typename ValueType>
inline MessageQueueGenericValueType<ValueType>::~MessageQueueGenericValueType()
{
}

//-----------------------------------------------------------------------

template<typename ValueType>
inline const ValueType & MessageQueueGenericValueType<ValueType>::getValue() const
{
	return m_value;
}

//-----------------------------------------------------------------------

#endif	// _INCLUDED_MessageQueueGenericValueType_H
