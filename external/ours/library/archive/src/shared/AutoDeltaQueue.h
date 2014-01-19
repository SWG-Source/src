
#ifndef	_INCLUDED_AutoDeltaQueue_H
#define	_INCLUDED_AutoDeltaQueue_H

//-----------------------------------------------------------------------

#include "AutoDeltaByteStream.h"
#include <list>

namespace Archive {

//-----------------------------------------------------------------------

/**
	@brief an auto-synchronization list template

	The AutoDeltaQueue implements a list container that knows when and what
	has changed in the list. The list may be added just like any other
	AutoDeltaVariableBase -- invoke addVariable(queueInstance)

	This particular AutoDeltaQueue is implemented as a list, though
	the interface does not require it to be implemented as such.

	@see AutoDeltaVariableBase
*/
template<typename ValueType>
class AutoDeltaQueue: public AutoDeltaContainer
{
public: // methods

	typedef std::list<ValueType> QueueType;
	typedef typename QueueType::iterator iterator;
	typedef typename QueueType::const_iterator const_iterator;

	AutoDeltaQueue();
	AutoDeltaQueue(AutoDeltaQueue const &source);
	~AutoDeltaQueue();

	void            clear();
	void            clearDelta() const;
	iterator        find(ValueType const &value);
	const_iterator  find(ValueType const &value) const;
	void            erase(iterator i);
	void            erase(int position);
	bool            empty() const;
	const bool      isDirty() const;
	void            pack(ByteStream &target) const;
	void            packDelta(ByteStream &target) const;
	const size_t    size() const;
	void            unpack(ReadIterator &source);
	void            unpackDelta(ReadIterator &source);

	void            push(ValueType const &value);
	void            pop();

	iterator begin();
	const_iterator begin() const;
	iterator end();
	const_iterator end() const;

private: // members
	struct ModifyCommand
	{
		enum Commands
		{
			PUSH,
			ERASE
		};
		unsigned char cmd;
		ValueType value;
		int position;
	};

	QueueType                    container;
	size_t                       baselineCommandCount;
	mutable std::vector<ModifyCommand> changes;

	AutoDeltaQueue &operator=(AutoDeltaQueue const &);
};

//-----------------------------------------------------------------------

template<typename ValueType>
inline AutoDeltaQueue<ValueType>::AutoDeltaQueue() :
AutoDeltaContainer(),
container(),
baselineCommandCount(0),
changes()
{
}

//-----------------------------------------------------------------------

template<typename ValueType>
inline AutoDeltaQueue<ValueType>::AutoDeltaQueue(const AutoDeltaQueue<ValueType> &source) :
AutoDeltaContainer(),
container(source.container),
baselineCommandCount(0),
changes()
{
}

//-----------------------------------------------------------------------

template<typename ValueType>
inline AutoDeltaQueue<ValueType>::~AutoDeltaQueue()
{
}

//-----------------------------------------------------------------------

template<typename ValueType>
inline typename AutoDeltaQueue<ValueType>::iterator AutoDeltaQueue<ValueType>::begin()
{
	return container.begin();
}

//-----------------------------------------------------------------------

template<typename ValueType>
inline typename AutoDeltaQueue<ValueType>::const_iterator AutoDeltaQueue<ValueType>::begin() const
{
	return container.begin();
}

//-----------------------------------------------------------------------

template<typename ValueType>
inline void AutoDeltaQueue<ValueType>::clear()
{
	//@todo: this is very inefficient!
	while (begin() != end())
	{
		erase(begin());
	}
}


//-----------------------------------------------------------------------

template<typename ValueType>
inline void AutoDeltaQueue<ValueType>::clearDelta() const
{
	changes.clear();
}

//-----------------------------------------------------------------------

template<typename ValueType>
inline typename AutoDeltaQueue<ValueType>::iterator AutoDeltaQueue<ValueType>::end()
{
	return container.end();
}

//-----------------------------------------------------------------------

template<typename ValueType>
inline typename AutoDeltaQueue<ValueType>::const_iterator AutoDeltaQueue<ValueType>::end() const
{
	return container.end();
}

//-----------------------------------------------------------------------

template<typename ValueType>
inline void AutoDeltaQueue<ValueType>::erase(iterator i)
{
	if (i != container.end())
	{
		ModifyCommand c;
		c.cmd = ModifyCommand::ERASE;
		c.position = std::distance(container.begin(), i);
		changes.push_back(c);
		++baselineCommandCount;
		container.erase(i);
		touch();
	}
}

//-----------------------------------------------------------------------

template<typename ValueType>
inline void AutoDeltaQueue<ValueType>::erase(int position)
{
	if (position < static_cast<int>(size()))
	{
		ModifyCommand c;
		c.cmd = ModifyCommand::ERASE;
		c.position = position;
		changes.push_back(c);
		++baselineCommandCount;
		iterator i = begin();
		std::advance(i, position);
		container.erase(i);
		touch();
	}
}

//-----------------------------------------------------------------------

template<typename ValueType>
inline void AutoDeltaQueue<ValueType>::pop()
{
	if (!empty())
	{
		ModifyCommand c;
		c.cmd = ModifyCommand::ERASE;
		c.position = 0;
		changes.push_back(c);
		++baselineCommandCount;
		container.pop_front();
		touch();
	}
}

//-----------------------------------------------------------------------

template<typename ValueType>
inline bool AutoDeltaQueue<ValueType>::empty() const
{
	return container.empty();
}

//-----------------------------------------------------------------------

template<typename ValueType>
inline typename AutoDeltaQueue<ValueType>::iterator AutoDeltaQueue<ValueType>::find(ValueType const &value)
{
	return container.find(value);
}

//-----------------------------------------------------------------------

template<typename ValueType>
inline typename AutoDeltaQueue<ValueType>::const_iterator AutoDeltaQueue<ValueType>::find(ValueType const &value) const
{
	return container.find(value);
}

//----------------------------------------------------------------------

template<typename ValueType>
inline const bool AutoDeltaQueue<ValueType>::isDirty() const
{
	return (!changes.empty());
}

//-----------------------------------------------------------------------

template<typename ValueType>
inline void AutoDeltaQueue<ValueType>::pack(ByteStream &target) const
{
	const_iterator i;
	Archive::put(target, container.size());
	Archive::put(target, baselineCommandCount);
	unsigned char cmd;
	for (i = container.begin(); i != container.end(); ++i)
	{
		cmd = ModifyCommand::PUSH;
		Archive::put(target, cmd);
		put(target, (*i));
	}
}

//-----------------------------------------------------------------------

template<typename ValueType>
inline void AutoDeltaQueue<ValueType>::packDelta(ByteStream &target) const
{
	Archive::put(target, changes.size());
	Archive::put(target, baselineCommandCount);
	for (typename std::vector<ModifyCommand>::iterator i = changes.begin(); i != changes.end(); ++i)
	{
		const ModifyCommand &c = (*i);
		Archive::put(target, c.cmd);
		switch(c.cmd)
		{
		case ModifyCommand::PUSH:
			{
				put(target, c.value);
			}
			break;
		case ModifyCommand::ERASE:
			{
				put(target, c.position);
			}
			break;
		}
	}
	clearDelta();
}

//-----------------------------------------------------------------------

template<typename ValueType>
inline const size_t AutoDeltaQueue<ValueType>::size() const
{
	return container.size();
}

//-----------------------------------------------------------------------

template<typename ValueType>
inline void AutoDeltaQueue<ValueType>::push(ValueType const &value)
{
	ModifyCommand c;
	c.cmd = ModifyCommand::PUSH;
	c.value = value;
	container.push_back(value);
	touch();
	changes.push_back(c);
	++baselineCommandCount;
}

//-----------------------------------------------------------------------

template<typename ValueType>
inline void AutoDeltaQueue<ValueType>::unpack(ReadIterator &source)
{
	using Archive::get;

	// unpacking baseline data
	container.clear();
	clearDelta();

	ModifyCommand c;
	size_t commandCount;

	Archive::get(source, commandCount);
	Archive::get(source, baselineCommandCount);

	for (size_t i = 0; i < commandCount; ++i)
	{
		Archive::get(source, c.cmd);
		assert(c.cmd == ModifyCommand::PUSH); // only push valid for pack/unpack
		get(source, c.value);
		container.push_back(c.value);
	}
}

//-----------------------------------------------------------------------

template<typename ValueType>
inline void AutoDeltaQueue<ValueType>::unpackDelta(ReadIterator &source)
{
	using Archive::get;
	ModifyCommand c;
	size_t skipCount, commandCount, targetBaselineCommandCount;

	Archive::get(source, commandCount);
	Archive::get(source, targetBaselineCommandCount);

	skipCount = commandCount+baselineCommandCount-targetBaselineCommandCount;

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
		switch (c.cmd)
		{
		case ModifyCommand::PUSH:
			{
				get(source, c.value);
			}
			break;
		case ModifyCommand::ERASE:
			{
				get(source, c.position);
			}
			break;
		default:
			assert(false);
			break;
		}
	}
	for ( ; i < commandCount; ++i)
	{
		Archive::get(source, c.cmd);
		switch (c.cmd)
		{
		case ModifyCommand::PUSH:
			{
				get(source, c.value);
				AutoDeltaQueue::push(c.value);
			}
			break;
		case ModifyCommand::ERASE:
			{
				get(source, c.position);
				AutoDeltaQueue::erase(c.position);
			}
			break;
		default:
			assert(false); // unknown command
			break;
		}
	}
}

//-----------------------------------------------------------------------

} //namespace Archive

#endif	// _INCLUDED__AutoDeltaQueue_H
