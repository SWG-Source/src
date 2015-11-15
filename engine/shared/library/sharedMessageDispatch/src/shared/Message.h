// ======================================================================
//
// Message.h
// copyright 2001 Sony Online Entertainment
//
// ======================================================================

#ifndef	INCLUDED_Message_H
#define	INCLUDED_Message_H

// ======================================================================

namespace MessageDispatch
{
	/**
		@brief A base class defining a common interface for Message object
		handling.

		The MessageBase is a pure virtual class. All Message objects
		handled by Emitter and Receiver objects derive from the MessageBase.

		@see Message
		@author Justin Randall
	*/
	class MessageBase
	{
	public:
		explicit MessageBase(const char * const typeName);
		explicit MessageBase(const unsigned long int type);
		virtual ~MessageBase();
		static const unsigned long int makeMessageTypeFromString(const char * const id);
		const unsigned long int        getType() const;
		const bool                     isType(const char * const typeName) const;
		void                           setType(const unsigned long int newType);
		void                           setType(const char * const typeName);
	private:
		MessageBase();
		unsigned long int   type;
	};

	/**
		@brief Message template

		Builds a message from some user supplied class.

		@author Justin Randall
	*/
	template<class ValueType>
	class Message : public MessageBase
	{
	public:
		explicit           Message(const char * const typeName="");
		explicit           Message(const unsigned long int type);
											 Message(const char * const typeName, const ValueType & value);
											 Message(const unsigned long int type, const ValueType & value);
		explicit           Message(const Message & source);
		virtual            ~Message();
		Message &          operator = (const Message & rhs);
		const ValueType &  getValue() const;
		void               setValue(const ValueType & rhs);
	private:
		ValueType          value;
	};

	//---------------------------------------------------------------------
	/**
		@brief get the type of Message

		@return the type of the Message. The type is an STL hash of the
		type name.
	*/
	inline const unsigned long int MessageBase::getType() const
	{
		return type;
	}

	//-----------------------------------------------------------------------

	inline const bool MessageBase::isType(const char * const typeName) const
	{
		return (makeMessageTypeFromString(typeName) == type);
	}

	//-----------------------------------------------------------------------
	/**
		@brief set or reset the type identifier
	*/
	inline void MessageBase::setType(const unsigned long newType)
	{
		type = newType;
	}

	//-----------------------------------------------------------------------
	/**
		@brief set or reset the type identifier
	*/
	inline void MessageBase::setType(const char * const typeName)
	{
		setType(makeMessageTypeFromString(typeName));
	}

	//-----------------------------------------------------------------------
	/**
		@brief construct a named message

		@param typeName     A string describing the type of message. This
												value is hashed (using the STL hash function) 
												the result is used for the MessageBase type name.
		@author Justin Randall
	*/
	template<class ValueType>
	inline Message<ValueType>::Message(const char * const typeName) :
	MessageBase(typeName),
	value()
	{
	}

	template<class ValueType>
	inline Message<ValueType>::Message(const unsigned long int type) :
	MessageBase(type),
	value()
	{
	}

	//---------------------------------------------------------------------
	/**
		@brief Construct a named message and initialize it to the 
		supplied ValueType value.

		@param typeName     A string describing the type of message. This
												value is hashed (using the STL hash function) 
												the result is used for the MessageBase type name.
		@newValue           Some value to initialize the message object to.

		@author Justin Randall
	*/
	template<class ValueType>
	inline Message<ValueType>::Message(const char * const typeName, const ValueType & newValue) :
	MessageBase(typeName),
	value(newValue)
	{
	}

	template<class ValueType>
	inline Message<ValueType>::Message(const unsigned long int type, const ValueType & newValue) :
	MessageBase(type),
	value(newValue)
	{
	}

	//---------------------------------------------------------------------
	/**
		@brief Message copy constructor

		Performs a copy of the message type and value members.

		@author Justin Randall
	*/
	template<class ValueType>
	inline Message<ValueType>::Message(const Message<ValueType> & source) :
	MessageBase(source.getType()),
	value(source.getValue())
	{
	}

	//---------------------------------------------------------------------
	/**
		@brief destroy the Message object

		Doesn't do anything special.

		@author Justin Randall
	*/
	template<class ValueType>
	inline Message<ValueType>::~Message()
	{
	}

	//---------------------------------------------------------------------
	/**
		@brief Assign the souce ValueType to the value member

		@param source   the new value for the Message::value member

		@author Justin Randall
	*/
	template<class ValueType>
	inline void Message<ValueType>::setValue(const ValueType & source)
	{
		value = source;
	}

	//---------------------------------------------------------------------
	/**
		@brief return the Message::value member

		@return the value of the value member
	*/
	template<class ValueType>
	inline const ValueType & Message<ValueType>::getValue() const
	{
		return value;
	}


}//namespace MessageDispatch

// ======================================================================

#endif	// _Message_H
