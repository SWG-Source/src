
//-----------------------------------------------------------------------
#include "FirstArchive.h"

#include "Archive.h"
#include "ByteStream.h"

#ifdef _WIN32
#pragma warning (disable: 4702)
#endif

#include "AutoDeltaByteStream.h"

namespace Archive {

//-----------------------------------------------------------------------

OnDirtyCallbackBase::OnDirtyCallbackBase()
{
}

//-----------------------------------------------------------------------

OnDirtyCallbackBase::~OnDirtyCallbackBase()
{
}

//-----------------------------------------------------------------------
/**
	@brief construct a AutoDeltaByteStream

	@author Justin Randall
*/
AutoDeltaByteStream::AutoDeltaByteStream() :
	AutoByteStream(),
	dirtyList(),
	onDirtyCallback(0)
{
}

//-----------------------------------------------------------------------
/**
	@brief destroy a AutoDeltaByteStream
	
	@author Justin Randall
*/
AutoDeltaByteStream::~AutoDeltaByteStream()
{
	onDirtyCallback = 0;
}

//-----------------------------------------------------------------------

void AutoDeltaByteStream::addOnDirtyCallback(OnDirtyCallbackBase * newCallback)
{
	onDirtyCallback = newCallback;
}

//-----------------------------------------------------------------------
/**
	@brief add a member to the dirty list

	This protected support method helps a AutoDeltaByteStream identify
	dirty variables.

	@author Justin Randall
*/
void AutoDeltaByteStream::addToDirtyList(AutoDeltaVariableBase * var)
{
	dirtyList.insert(var); //lint !e534 // ignoring iterator returned from insert
	if (onDirtyCallback)
	{
		onDirtyCallback->onDirty();
	}
}

//---------------------------------------------------------------------
/**
	@brief Add a new variable to the AutoDeltaByteStream.

	This method leverages the AutoByteStream member list ot identify
	values tracked by the AutoByteStream. Additionally, the variable
	index is set so that it may identify itself when it is used
	in a non-const manner, and it receives a reference to it's owner
	AutoDeltaByteStream.

	@author Justin Randall
*/
void AutoDeltaByteStream::addVariable(AutoDeltaVariableBase & var)
{
	var.setIndex(static_cast<unsigned short int>(members.size()));
	var.setOwner(this);
	AutoByteStream::addVariable(var);
}

//---------------------------------------------------------------------

const unsigned int AutoDeltaByteStream::getItemCount() const
{
	unsigned short int count = 0;

	if (!dirtyList.empty())
	{
		for (std::set<void *>::const_iterator i = dirtyList.begin(); i != dirtyList.end(); ++i)
		{
			AutoDeltaVariableBase * const v = reinterpret_cast<AutoDeltaVariableBase *>(*i);
			if (v->isDirty())
				++count;
		}
	}

	return count;
}

//-----------------------------------------------------------------------
/**
	@brief Pack values that have changed since the last time deltas
	were collected.

	@author Justin Randall
*/
void AutoDeltaByteStream::packDeltas(ByteStream & target) const
{
	unsigned short int const count = static_cast<unsigned short int>(getItemCount());

	// place count in archive
	Archive::put(target, count);
	
	if (count > 0)
	{
		for (std::set<void *>::const_iterator i = dirtyList.begin(); i != dirtyList.end(); ++i)
		{
			AutoDeltaVariableBase * const v = reinterpret_cast<AutoDeltaVariableBase *>(*i);
			if (v->isDirty())
			{
				put(target, v->getIndex());
				v->packDelta(target);
			}
		}
	}

	dirtyList.clear();
}

//-----------------------------------------------------------------------

void AutoDeltaByteStream::clearDeltas() const
{
	if (!dirtyList.empty())
	{
		for (std::set<void *>::const_iterator i = dirtyList.begin(); i != dirtyList.end(); ++i)
		{
			AutoDeltaVariableBase * const v = reinterpret_cast<AutoDeltaVariableBase *>(*i);
			v->clearDelta();
		}
		dirtyList.clear();
	}
}

//-----------------------------------------------------------------------

void AutoDeltaByteStream::removeOnDirtyCallback()
{
	onDirtyCallback = 0;
}

//---------------------------------------------------------------------
/**
	@brief receive a delta ByteStream and apply the new deltas. 

	Receive a delta ByteStream and apply the new deltas to the local
	object. The ByteStream MUST be created by another AutoDeltaByteStream!

	@author Justin Randall
*/
void AutoDeltaByteStream::unpackDeltas(ReadIterator & source)
{
	unsigned short int index;
	unsigned short int count;
	Archive::get(source, count);

	while (source.getSize())
	{
		get(source, index);
		AutoDeltaVariableBase * const v = reinterpret_cast<AutoDeltaVariableBase *>(members[index]);
		v->unpackDelta(source);
	}
}

//-----------------------------------------------------------------------
/**
	@brief constructor for an abstract AutoDeltaVariableBase object

	@author Justin Randall
*/
AutoDeltaVariableBase::AutoDeltaVariableBase() :
	AutoVariableBase(),
	index(0),
	owner(0)
{
}

//-----------------------------------------------------------------------
/**
	@brief desctructor for a AutoDeltaVariableBase or derivative
	object.

	@author Justin Randall
*/
AutoDeltaVariableBase::~AutoDeltaVariableBase()
{
	owner = 0;
}

//-----------------------------------------------------------------------

}//namespace Archive

