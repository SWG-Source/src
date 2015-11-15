
//---------------------------------------------------------------------
#include "FirstArchive.h"

#include "Archive.h"
#include "AutoByteStream.h"
#include "ByteStream.h"

namespace Archive {

//---------------------------------------------------------------------
/**
	@brief construct a AutoByteStream

	Invokes the default constructor for the members vector

	@see ByteStream

	@author Justin Randall
*/
AutoByteStream::AutoByteStream() :
members()
{
}

//---------------------------------------------------------------------
/**
	@brief destroy the AutoByteStream

	Doesn't do anything special.

	@see ByteStream

	@author Justin Randall
*/
AutoByteStream::~AutoByteStream()
{
}

//---------------------------------------------------------------------
/**
	@brief Add a AutoVariable to a list of data to be automatically
  packed or unpacked.

	Add the instance of a AutoVariableBase derivative to the 
	AutoByteStream's list of values to be packed and unpacked 
	automatically. The AutoByteStream will handle correct packing
	and unpacking order as long as the variables are added in
	order. Typically, these variables are added during construction
	of some AutoByteStream derived class. 

	Once a variable has been added, it cannot be removed.

	@param newVariable  A reference to the instance of the 
	                    AutoVariableBase that will be added to the
						AutoByteStream's list of tracked values.

	@see AutoByteStream
	@see AutoVariableBase
	@see AutoVariable
	@see ByteStream

	@author Justin Randall
*/
void AutoByteStream::addVariable(AutoVariableBase & newVariable)
{
	members.push_back(&newVariable);
}
//-----------------------------------------------------------------------

const unsigned int AutoByteStream::getItemCount() const
{
	return members.size();
}

//---------------------------------------------------------------------
/**
	@brief Pack the values (AutoVariableBase or derivatives) into the
		ByteStream Buffer

	When the pack method is invoked, all of the members added with
	addVariable are placed, in the order they were added, into the
	ByteStream's buffer. The ByteStream returns a reference to itself
	after the pack operation has completed.

	@see AutoVariableBase::pack
	@see ByteStream::pack
	@see ByteStream
	@see AutoByteStream
	@see AutoVariable

	@return a reference to this AutoByteStream

	@author Justin Randall
*/
void AutoByteStream::pack(ByteStream & target) const
{
	std::vector<AutoVariableBase *>::const_iterator i;
	unsigned short packedSize=static_cast<unsigned short>(members.size());
	Archive::put(target,packedSize);
	for(i = members.begin(); i != members.end(); ++i)
	{
		(*i)->pack(target);
	}
}

//---------------------------------------------------------------------
/**
	@brief Restore values from an ByteStream buffer

	Values are restored from the source ByteStream's buffer in the order
	that this ByteStream's members were added (using addVariable).

	@param source  The ByteStream containing the source data starting at
	               the ByteStream's current read position.

	@see AutoByteStream::pack
	@see AutoByteStream
	@see ByteStream::unpack
	@see ByteStream::pack
	@see ByteStream
	@see AutoVariableBase::unpack
	@see AutoVariableBase

	@author Justin Randall
*/
void AutoByteStream::unpack(ReadIterator & source)
{
	std::vector<AutoVariableBase *>::iterator i;
	unsigned short packedSize;
	Archive::get(source,packedSize);
	for(i = members.begin(); i != members.end(); ++i)
	{
		(*i)->unpack(source);
	}
}

//---------------------------------------------------------------------
/**
	@brief construct an AutoVariableBase object

	The AutoVariableBase class is pure virtual. This is never directly
	invoked.

	@author Justin Randall
*/
AutoVariableBase::AutoVariableBase()
{
}

//---------------------------------------------------------------------
/**
	@brief AutoVariableBase dtor

	Defined, but is pure virtual
	
	@author Justin Randall
*/
AutoVariableBase::~AutoVariableBase()
{
}

//----------------------------------------------------------------------

}//namespace Archive
