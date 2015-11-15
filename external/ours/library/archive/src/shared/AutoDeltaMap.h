
#ifndef	_INCLUDED_AutoDeltaMap_H
#define	_INCLUDED_AutoDeltaMap_H

//-----------------------------------------------------------------------

#include "AutoDeltaByteStream.h"
#include <map>

//-----------------------------------------------------------------------

namespace Archive {

//-----------------------------------------------------------------------
/**
	@brief an auto-synchronization map template

	The AutoDeltaMap implements a map container that knows when and what
	has changed in the map. The map may be added just like any other
	AutoDeltaVariableBase -- invoke addVariable(mapInstance)

	This particular AutoDeltaMap is implemented as a map, though
	the interface does not require it to be implemented as such.

	@see AutoDeltaVariableBase

	@author Justin Randall
*/
template<class KeyType, typename ValueType, typename ObjectType=DefaultObjectType>
class AutoDeltaMap : public AutoDeltaContainer
{
public:
	struct Command
	{
		enum Commands
		{
			ADD,
			ERASE,
			SET
		};
		unsigned char cmd;
		KeyType key;
		ValueType value;
	};

public: // methods

	typedef std::map<KeyType, ValueType> MapType;
	typedef typename MapType::const_iterator const_iterator;

	AutoDeltaMap();
	AutoDeltaMap(const AutoDeltaMap & source);
	~AutoDeltaMap();

	void            clear();
	void            clearDelta() const;
	const_iterator  erase(const KeyType & key);
	const_iterator  erase(const_iterator & i);
	bool            empty() const;
	const_iterator  find(const KeyType & key) const;
	const_iterator  lower_bound(const KeyType & key) const;
	const MapType & getMap() const;
	std::pair<const_iterator, bool> insert(const KeyType & key, const ValueType & value);
	const bool      isDirty() const;
	void            pack(ByteStream & target) const;
	void            packDelta(ByteStream & target) const;
	void            set(const KeyType & key, const ValueType & value);
	const size_t    size() const;
	void            unpack(ReadIterator & source);
	void            unpackDelta(ReadIterator & source);
	static void     pack(ByteStream & target, const std::vector<Command> & data);
	static void     unpack(ReadIterator & source, std::vector<Command> & data);
	static void     unpackDelta(ReadIterator & source, std::vector<Command> & data);


	const_iterator begin() const;
	const_iterator end() const;

	void setOnErase   (ObjectType * owner, void (ObjectType::*onErase)(const KeyType &, const ValueType &));
	void setOnInsert  (ObjectType * owner, void (ObjectType::*onInsert)(const KeyType &, const ValueType &));
	void setOnSet     (ObjectType * owner, void (ObjectType::*onSet)(const KeyType &, const ValueType &, const ValueType &));

private:
	AutoDeltaMap &operator=(const AutoDeltaMap &);

	void onErase(const KeyType &, const ValueType &);
	void onInsert(const KeyType &, const ValueType &);
	void onSet(const KeyType &, const ValueType &, const ValueType &);

	MapType                      container;
	size_t                       baselineCommandCount;
	mutable std::vector<Command> changes;
	std::pair<ObjectType *, void (ObjectType::*)(const KeyType &, const ValueType &)> *onEraseCallback;
	std::pair<ObjectType *, void (ObjectType::*)(const KeyType &, const ValueType &)> *onInsertCallback;
	std::pair<ObjectType *, void (ObjectType::*)(const KeyType &, const ValueType &, const ValueType &)> *onSetCallback;
};

//-----------------------------------------------------------------------
/**
	@brief default constructor, inits base class, container and container
	copy

	@author Justin Randall
*/
template<class KeyType, typename ValueType, typename ObjectType>
inline AutoDeltaMap<KeyType, ValueType, ObjectType>::AutoDeltaMap() :
AutoDeltaContainer(),
container(),
baselineCommandCount(0),
changes(),
onEraseCallback(0),
onInsertCallback(0),
onSetCallback(0)
{
}

//-----------------------------------------------------------------------
/**
	@brief copy constructor

	@author Justin Randall
*/
template<class KeyType, typename ValueType, typename ObjectType>
inline AutoDeltaMap<KeyType, ValueType, ObjectType>::AutoDeltaMap(const AutoDeltaMap<KeyType, ValueType, ObjectType> & source) :
AutoDeltaContainer(),
container(source.container),
baselineCommandCount(0),
changes(),
onEraseCallback(0),
onInsertCallback(0),
onSetCallback(0)
{
}

//-----------------------------------------------------------------------
/**
	@brief dtor

	@author Justin Randall
*/
template<class KeyType, typename ValueType, typename ObjectType>
inline AutoDeltaMap<KeyType, ValueType, ObjectType>::~AutoDeltaMap()
{
	delete onEraseCallback;
	delete onInsertCallback;
	delete onSetCallback;
}

//-----------------------------------------------------------------------
/**
	@brief like std::map::begin(), returns a const_iterator to the first
	element in the map.

	Wraps a call to std::map<ValueType>::begin

	@return const_iterator to the first element of the map

	@author Justin Randall
*/
template<class KeyType, typename ValueType, typename ObjectType>
inline typename AutoDeltaMap<KeyType, ValueType, ObjectType>::const_iterator AutoDeltaMap<KeyType, ValueType, ObjectType>::begin() const
{
	return container.begin();
}

//-----------------------------------------------------------------------
/**
	@brief support routine to clear the map.

	@author Steve Jakab
*/
template<class KeyType, typename ValueType, typename ObjectType>
inline void AutoDeltaMap<KeyType, ValueType, ObjectType>::clear()
{
	//@todo: this is very inefficient!
	while (begin() != end())
	{
		const_iterator i = begin();
		erase(i);
	}
}


//-----------------------------------------------------------------------
/**
	@brief support routine to clear deltas in the map.

	@author Justin Randall
*/
template<class KeyType, typename ValueType, typename ObjectType>
inline void AutoDeltaMap<KeyType, ValueType, ObjectType>::clearDelta() const
{
	changes.clear();
}

//-----------------------------------------------------------------------
/**
	@brief like std::map::end(), returns a const_iterator to the end
	of the map.

	@return a const_iterator to the end of the map

	@author Justin Randall
*/
template<class KeyType, typename ValueType, typename ObjectType>
inline typename AutoDeltaMap<KeyType, ValueType, ObjectType>::const_iterator AutoDeltaMap<KeyType, ValueType, ObjectType>::end() const
{
	return container.end();
}

//-----------------------------------------------------------------------
/**
	@brief invokes map::erase(KeyType &)

	Removes an element from the map

	@param key  A keyType describing some element of the map

	@author Justin Randall
*/
template<class KeyType, typename ValueType, typename ObjectType>
inline typename AutoDeltaMap<KeyType, ValueType, ObjectType>::const_iterator AutoDeltaMap<KeyType, ValueType, ObjectType>::erase(const KeyType & key)
{
	typename std::map<KeyType, ValueType>::iterator f(container.find(key));
	if (f != container.end())
	{
		Command c;
		c.cmd = Command::ERASE;
		c.key = (*f).first;
		c.value = (*f).second;
		changes.push_back(c);
		++baselineCommandCount;
		container.erase(f++);
		touch();
		onErase(key, c.value);
	}
	return f;
}

//-----------------------------------------------------------------------
/**
	@brief invokes map::erase(const_iterator &)

	Removes and element from the map

	@param i  a const_iterator describing an element in the map

	@author Justin Randall
*/
template<class KeyType, typename ValueType, typename ObjectType>
inline typename AutoDeltaMap<KeyType, ValueType, ObjectType>::const_iterator AutoDeltaMap<KeyType, ValueType, ObjectType>::erase(const_iterator & i)
{
	return erase((*i).first);
}

//-----------------------------------------------------------------------
/**
	@brief invokes map::empty()

	Returns true if the map is empty.

	@author Acy Stapp
*/
template<class KeyType, typename ValueType, typename ObjectType>
inline bool AutoDeltaMap<KeyType, ValueType, ObjectType>::empty() const
{
	return container.empty();
}

//-----------------------------------------------------------------------
/**
	@brief return a map::<ValueType>::const_iterator

	@author Justin Randall
*/
template<class KeyType, typename ValueType, typename ObjectType>
inline typename AutoDeltaMap<KeyType, ValueType, ObjectType>::const_iterator AutoDeltaMap<KeyType, ValueType, ObjectType>::find(const KeyType & key) const
{
	return container.find(key);
}

// ----------------------------------------------------------------------

template<class KeyType, typename ValueType, typename ObjectType>
inline typename AutoDeltaMap<KeyType, ValueType, ObjectType>::const_iterator AutoDeltaMap<KeyType, ValueType, ObjectType>::lower_bound(const KeyType & key) const
{
	return container.lower_bound(key);
}

//----------------------------------------------------------------------

template<class KeyType, typename ValueType, typename ObjectType>
inline const typename AutoDeltaMap<KeyType, ValueType, ObjectType>::MapType & AutoDeltaMap<KeyType, ValueType, ObjectType>::getMap() const
{
	return container;
}

//-----------------------------------------------------------------------

template<class KeyType, typename ValueType, typename ObjectType>
#if defined(linux) || _MSC_VER >= 1300
inline std::pair<typename AutoDeltaMap<KeyType, ValueType, ObjectType>::const_iterator, bool> AutoDeltaMap<KeyType, ValueType, ObjectType>::insert(const KeyType & key, const ValueType & value)
#else
inline std::pair<AutoDeltaMap<KeyType, ValueType, ObjectType>::const_iterator, bool> AutoDeltaMap<KeyType, ValueType, ObjectType>::insert(const KeyType & key, const ValueType & value)
#endif
{
	typedef typename std::map<KeyType, ValueType>::const_iterator iter;
	typedef std::pair<iter, bool> insertResult;

	iter i = container.find(key);
	if (i != container.end())
		return std::make_pair(i, false);

	Command c;
	c.cmd = Command::ADD;
	c.key = key;
	c.value = value;
	insertResult result = container.insert(std::make_pair(key, value));
	touch();
	onInsert(key, value);
	changes.push_back(c);
	++baselineCommandCount;
	return result;
}

//-----------------------------------------------------------------------
/**
	@brief is the map dirty?

	If any operation has modified the map adds changes to its list,
	so if there are any changes, the map is dirty.

	@return true if the map is dirty
*/
template<class KeyType, typename ValueType, typename ObjectType>
inline const bool AutoDeltaMap<KeyType, ValueType, ObjectType>::isDirty() const
{
	return (!changes.empty());
}

//-----------------------------------------------------------------------
/**
	@brief pack the whole container

	Iterates through all elements in the container, packing them in
	the supplied ByteStream.
*/
template<class KeyType, typename ValueType, typename ObjectType>
inline void AutoDeltaMap<KeyType, ValueType, ObjectType>::pack(ByteStream & target) const
{
	typename std::map<KeyType, ValueType>::const_iterator i;
	Archive::put(target, container.size());
	Archive::put(target, baselineCommandCount);
	unsigned char cmd;
	for(i = container.begin(); i != container.end(); ++i)
	{
		cmd = Command::ADD;
		Archive::put(target, cmd);
		put(target, (*i).first);
		put(target, (*i).second);
	}
}

//-----------------------------------------------------------------------
/**
	@brief Pack the whole container from a list of updates, without
	instantiating the container.

	This converts a vector of commands to a stream of bytes.  This
	byte stream can be unpacked into an AutoDeltaMap.

	This function is useful when reading the data from a file or database.
*/
template<class KeyType, typename ValueType, typename ObjectType>
inline void AutoDeltaMap<KeyType, ValueType, ObjectType>::pack(ByteStream & target, const std::vector<Command> &data)
{
	Archive::put(target, data.size());
	Archive::put(target, static_cast<size_t>(0)); // baselineCommandCount
	for(typename std::vector<Command>::const_iterator c(data.begin()); c != data.end(); ++c)
	{
		assert(c->cmd == Command::ADD); // only add is valid in packing the whole container
		Archive::put(target, c->cmd);
		put(target, c->key);
		put(target, c->value);
	}
}

//-----------------------------------------------------------------------
/**
	@brief only pack elements which have changed

	This runs through the list of changes which has already been
	generated, and packs them.

	Each add, remove or set are prefixed with a CONTAINER_SET or
	CONTAINER_ERASE command. (adds and sets both use CONTAINER_SET).

	@param target   a ByteStream object that will receive the delta package
	                at its current write position

	@author Justin Randall
*/
template<class KeyType, typename ValueType, typename ObjectType>
inline void AutoDeltaMap<KeyType, ValueType, ObjectType>::packDelta(ByteStream & target) const
{
	Archive::put(target, changes.size());
	Archive::put(target, baselineCommandCount);
	for (typename std::vector<Command>::iterator i = changes.begin(); i != changes.end(); ++i)
	{
		const Command & c = (*i);
		Archive::put(target, c.cmd);
		switch (c.cmd)
		{
			case Command::ADD:
			case Command::SET:
			case Command::ERASE:
			{
				put(target, c.key);
				put(target, c.value);
			}
			break;
			default:
				assert(false); // unknown command put INTO the archive!!!! THis will
				// cause a crash when unpacking on a remote system

		}
	}
	clearDelta();
}

//-----------------------------------------------------------------------
/**
	@brief get the number of elements in the map

	@return the number of elements in the map
*/
template<class KeyType, typename ValueType, typename ObjectType>
inline const size_t AutoDeltaMap<KeyType, ValueType, ObjectType>::size() const
{
	return container.size();
}

//-----------------------------------------------------------------------
/**
	@brief set an element in the map

	If the key does not exist in the map, then an add is queued (which
	will trigger a delta), the value is added to the map, and touch() is
	called place the container on the AutoDeltaByteStream's dirty list.

	If they key exists, and the values differ, and if the key is not in the
	changes set, the old value is stored in changes, the new value is in
	the current container's map and the container invokes touch() to
	place itself on the AutoDeltaByteStream's dirty list.

	@param key     Unique identifier for the value
	@param value   value associated with the unique key
*/
template<class KeyType, typename ValueType, typename ObjectType>
inline void AutoDeltaMap<KeyType, ValueType, ObjectType>::set(const KeyType & key, const ValueType & value)
{
	Command c;
	typename std::map<KeyType, ValueType>::iterator i = container.find(key);
	if (i == container.end())
	{
		c.cmd = Command::ADD;
		c.key = key;
		c.value = value;
		container[key] = value;
		touch();
		onInsert(key, value);
		changes.push_back(c);
		++baselineCommandCount;
	}
	else
	{
		// did it really change?
		if ((*i).second != value)
		{
			c.cmd = Command::SET;
			c.key = key;
			c.value = value;
			ValueType oldValue = i->second;
			// set it!
			i->second = value;
			touch();
			onSet(key, oldValue, value);
			changes.push_back(c);
			++baselineCommandCount;
		}
	}
}

//-----------------------------------------------------------------------
/**
	@brief rebuild the container from a byte stream

	The container first determines the number of changes that will be
	applied, which is the first element in the ByteStream.

	Then, for each element, the command is read (CONTAINER_SET or
	CONTAINER_ERASE). If it is a CONTAINER_SET, then the key and value
	are read from the byte stream and applied to the container. If the
	command is CONTAINER_ERASE, the key is retrieved, and the key/value
	pair is erased from the container.

	@param source   a ByteStream containing the delta or baseline package.
*/
template<class KeyType, typename ValueType, typename ObjectType>
inline void AutoDeltaMap<KeyType, ValueType, ObjectType>::unpack(ReadIterator & source)
{
	// unpacking baseline data
	container.clear();
	clearDelta();

	Command c;
	size_t commandCount;

	Archive::get(source, commandCount);
	Archive::get(source, baselineCommandCount);

	for (size_t i = 0; i < commandCount; ++i)
	{
		Archive::get(source, c.cmd);
		assert(c.cmd == Command::ADD); // only add is valid in unpack
		get(source, c.key);
		get(source, c.value);
		container[c.key] = c.value;
		onInsert(c.key, c.value);
	}
}

//-----------------------------------------------------------------------
/**
	@brief Get the elements of the container without instantiating it.

	This reads the byte stream like the non-static unpack(), but it constructs
	a vector of commands that would build the container.

	This is useful for writing the data to an external data store, e.g.
	a file or a database.

	@param source   a ByteStream containing the delta or baseline package.
*/
template<class KeyType, typename ValueType, typename ObjectType>
inline void AutoDeltaMap<KeyType, ValueType, ObjectType>::unpack(ReadIterator & source, std::vector<Command> & data)
{
	// unpacking baseline data

	Command c;
	size_t commandCount;
	size_t baselineCommandCount;

	Archive::get(source, commandCount);
	Archive::get(source, baselineCommandCount);

	for (size_t i = 0; i < commandCount; ++i)
	{
		Archive::get(source, c.cmd);
		assert(c.cmd == Command::ADD); // only add is valid in unpack
		get(source, c.key);
		get(source, c.value);
		data.push_back(c);
	}
}

//-----------------------------------------------------------------------
/**
	@brief update the container from a byte stream
*/
template<class KeyType, typename ValueType, typename ObjectType>
inline void AutoDeltaMap<KeyType, ValueType, ObjectType>::unpackDelta(ReadIterator & source)
{
	Command c;
	size_t skipCount, commandCount, targetBaselineCommandCount;

	Archive::get(source, commandCount);
	Archive::get(source, targetBaselineCommandCount);

	// if (commandCount+baselineCommandCount) < targetBaselineCommandCount, it
	// means that we have missed some changes and are behind; when this happens,
	// catch up by applying all the deltas that came in, and set
	// baselineCommandCount to targetBaselineCommandCount
	if ((commandCount+baselineCommandCount) > targetBaselineCommandCount)
		skipCount = commandCount+baselineCommandCount-targetBaselineCommandCount;
	else
		skipCount = 0;

	// If this fails, it means that the deltas we are receiving are relative to baselines
	// which are newer than what we currently have.  This usually means either we were not
	// observing an object for a time when deltas were sent, but aren't getting new
	// baselines, or our version of the container has been modified locally.
	if (skipCount > commandCount)
		skipCount = commandCount;

	size_t i;
	for (i = 0; i < skipCount; ++i)
	{
		Archive::get(source, c.cmd);
		get(source, c.key);
		get(source, c.value);
	}
	for ( ; i < commandCount; ++i)
	{
		Archive::get(source, c.cmd);
		switch(c.cmd)
		{
		case Command::ADD:
		case Command::SET:
			{
				get(source, c.key);
				get(source, c.value);
				AutoDeltaMap::set(c.key, c.value);
			}
			break;
		case Command::ERASE:
			{
				get(source, c.key);
				get(source, c.value);
				AutoDeltaMap::erase(c.key);
			}
			break;
		default:
			assert(false); // unknown command
			break;
		}
	}

	// if we are behind, catch up
	if (baselineCommandCount < targetBaselineCommandCount)
		baselineCommandCount = targetBaselineCommandCount;
}

//-----------------------------------------------------------------------
/**
	@brief Get the deltas to the container without instantiating it.

	This reads the byte stream like the non-static unpackDeltas(), but it constructs
	a vector of commands that would update the container.

	This is useful for writing the data to an external data store, e.g.
	a file or a database.

	@param source   a ByteStream containing the delta or baseline package.
*/
template<class KeyType, typename ValueType, typename ObjectType>
inline void AutoDeltaMap<KeyType, ValueType, ObjectType>::unpackDelta(ReadIterator & source, std::vector<Command> & data)
{
	Command c;
	size_t commandCount, targetBaselineCommandCount;

	Archive::get(source, commandCount);
	Archive::get(source, targetBaselineCommandCount);

	for (size_t i=0 ; i < commandCount; ++i)
	{
		Archive::get(source, c.cmd);
		switch(c.cmd)
		{
		case Command::ADD:
		case Command::SET:
		case Command::ERASE:
			{
				get(source, c.key);
				get(source, c.value);
			}
			break;
		default:
			assert(false); // unknown command
			break;
		}

		data.push_back(c);
	}
}

//-----------------------------------------------------------------------

template<class KeyType, typename ValueType, typename ObjectType>
inline void AutoDeltaMap<KeyType, ValueType, ObjectType>::onSet(const KeyType &key, const ValueType &oldValue, const ValueType &newValue)
{
	if (onSetCallback && onSetCallback->first)
	{
		ObjectType &owner = *onSetCallback->first;
		void (ObjectType::*cb)(const KeyType &, const ValueType &, const ValueType &) = onSetCallback->second;
		(owner.*cb)(key, oldValue, newValue);
	}
}

//-----------------------------------------------------------------------

template<class KeyType, typename ValueType, typename ObjectType>
inline void AutoDeltaMap<KeyType, ValueType, ObjectType>::onErase(const KeyType &key, const ValueType &value)
{
	if (onEraseCallback && onEraseCallback->first)
	{
		ObjectType &owner = *onEraseCallback->first;
		void (ObjectType::*cb)(const KeyType &, const ValueType &) = onEraseCallback->second;
		(owner.*cb)(key, value);
	}
}

//-----------------------------------------------------------------------

template<class KeyType, typename ValueType, typename ObjectType>
inline void AutoDeltaMap<KeyType, ValueType, ObjectType>::onInsert(const KeyType &key, const ValueType &value)
{
	if (onInsertCallback && onInsertCallback->first)
	{
		ObjectType &owner = *onInsertCallback->first;
		void (ObjectType::*cb)(const KeyType &, const ValueType &) = onInsertCallback->second;
		(owner.*cb)(key, value);
	}
}

//-----------------------------------------------------------------------

template<class KeyType, typename ValueType, typename ObjectType>
inline void AutoDeltaMap<KeyType, ValueType, ObjectType>::setOnErase(ObjectType * owner, void (ObjectType::*cb)(const KeyType &, const ValueType &))
{
	delete onEraseCallback;
	onEraseCallback = new std::pair<ObjectType *, void (ObjectType::*)(const KeyType &, const ValueType &)>;
	onEraseCallback->first = owner;
	onEraseCallback->second = cb;
}

//-----------------------------------------------------------------------

template<class KeyType, typename ValueType, typename ObjectType>
inline void AutoDeltaMap<KeyType, ValueType, ObjectType>::setOnInsert(ObjectType * owner, void (ObjectType::*cb)(const KeyType &, const ValueType &))
{
	delete onInsertCallback;
	onInsertCallback = new std::pair<ObjectType *, void (ObjectType::*)(const KeyType &, const ValueType &)>;
	onInsertCallback->first = owner;
	onInsertCallback->second = cb;
}

//-----------------------------------------------------------------------

template<class KeyType, typename ValueType, typename ObjectType>
inline void AutoDeltaMap<KeyType, ValueType, ObjectType>::setOnSet(ObjectType * owner, void (ObjectType::*cb)(const KeyType &, const ValueType &, const ValueType &))
{
	delete onSetCallback;
	onSetCallback = new std::pair<ObjectType *, void (ObjectType::*)(const KeyType &, const ValueType &, const ValueType &)>;
	onSetCallback->first = owner;
	onSetCallback->second = cb;
}

//-----------------------------------------------------------------------

}//namespace Archive

#endif	// _INCLUDED__AutoDeltaMap_H

