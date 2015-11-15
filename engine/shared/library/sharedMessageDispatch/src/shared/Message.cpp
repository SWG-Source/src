// ======================================================================
//
// FirstMessageDispatch.h
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#include "sharedMessageDispatch/FirstSharedMessageDispatch.h"
#include "sharedMessageDispatch/Message.h"

#include "sharedFoundation/LabelHash.h"
#include "sharedFoundation/Crc.h"
#include "sharedFoundation/Production.h"

#include <string>

namespace MessageDispatch
{

// ======================================================================

/**
	@brief build a message type to identify a message

	Returns an STL hash of the type name.
*/
const unsigned long int MessageBase::makeMessageTypeFromString(const char * const idString)
{
	unsigned long int result = 0;
#if PRODUCTION == 0
	result = LabelHash::hashLabel("MessageDispatch", idString);
#else
	result = Crc::calculate(idString);
#endif
	return result;
}

//---------------------------------------------------------------------
/**
	@brief construct a named message object

	The MessageBase is an abstract base class. Initializes the type
	member to the STL hash of the name string.

	@author Justin Randall
*/
MessageBase::MessageBase(const char * const typeName) :
type(0)
{
	type = makeMessageTypeFromString(typeName);
}

//---------------------------------------------------------------------
/**
	@brief construct a named message object

	The MessageBase is an abstract base class. Initializes the type
	the supplied type. Useful when the hash is cached.

	@author Justin Randall
*/
MessageBase::MessageBase(const unsigned long int newType) :
type(newType)
{
}

//---------------------------------------------------------------------
/**
	@brief destroy the MessageBase object

	Doesn't do anything special.

	@author Justin Randall
*/
MessageBase::~MessageBase()
{
}

// ======================================================================

} // namespace MessageDispatch
