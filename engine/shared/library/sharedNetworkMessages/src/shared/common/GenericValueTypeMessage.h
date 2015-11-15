// GenericValueTypeMessage.h
// Copyright 2000-02, Sony Online Entertainment Inc., all rights reserved. 
// Author: Justin Randall

#ifndef	_INCLUDED_GenericValueTypeMessage_H
#define	_INCLUDED_GenericValueTypeMessage_H

//-----------------------------------------------------------------------

#include "sharedNetworkMessages/GameNetworkMessage.h"

//-----------------------------------------------------------------------

class CrcString;

//-----------------------------------------------------------------------

template<typename ValueType>
class GenericValueTypeMessage : public GameNetworkMessage
{
public:
	GenericValueTypeMessage(const std::string & messageName, const ValueType & value);
	GenericValueTypeMessage(CrcString const &messageName, const ValueType & value);
	explicit  GenericValueTypeMessage  (Archive::ReadIterator & source);
	~GenericValueTypeMessage();

	const ValueType &  getValue  ()  const;

private:
	GenericValueTypeMessage & operator = (const GenericValueTypeMessage & rhs);
	GenericValueTypeMessage(const GenericValueTypeMessage & source);

	Archive::AutoVariable<ValueType>  value;
};

//-----------------------------------------------------------------------

template<typename ValueType>
GenericValueTypeMessage<ValueType>::GenericValueTypeMessage(const std::string & n, const ValueType & v) :
GameNetworkMessage(n),
value(v)
{
	addVariable(value);
}

//-----------------------------------------------------------------------

template<typename ValueType>
GenericValueTypeMessage<ValueType>::GenericValueTypeMessage(CrcString const &n, const ValueType & v) :
GameNetworkMessage(n),
value(v)
{
	addVariable(value);
}

//-----------------------------------------------------------------------

template<typename ValueType>
GenericValueTypeMessage<ValueType>::GenericValueTypeMessage(Archive::ReadIterator & source) :
GameNetworkMessage(""),
value()
{
	addVariable(value);
	unpack(source);
}

//-----------------------------------------------------------------------

template<typename ValueType>
GenericValueTypeMessage<ValueType>::~GenericValueTypeMessage()
{
}

//-----------------------------------------------------------------------
template<typename ValueType>
inline const ValueType & GenericValueTypeMessage<ValueType>::getValue() const
{
	return value.get();
}

//-----------------------------------------------------------------------

#endif	// _INCLUDED_GenericValueTypeMessage_H
