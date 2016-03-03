// ======================================================================
//
// AutoDeltaSet.h
//
// Copyright 2003 Sony Online Entertainment
//
// ======================================================================

#ifndef	_INCLUDED_AutoDeltaSet_H
#define	_INCLUDED_AutoDeltaSet_H

// ======================================================================

#include "AutoDeltaByteStream.h"

// ======================================================================

namespace Archive {

//-----------------------------------------------------------------------

template<typename ValueType, typename ObjectType = DefaultObjectType>
class AutoDeltaSet: public AutoDeltaContainer
{

public:
	typedef std::set<ValueType> SetType;
	typedef typename SetType::size_type size_type;
	typedef typename SetType::const_iterator const_iterator;
	typedef typename SetType::const_reverse_iterator const_reverse_iterator;

public:
	struct Command
	{
		enum
		{
			ERASE,
			INSERT,
			CLEAR
		};
		unsigned char cmd;
		ValueType value;
	};

public:
	AutoDeltaSet();
	~AutoDeltaSet();

	const_iterator         begin() const;
	const_iterator         end() const;
	const_reverse_iterator rbegin() const;
	const_reverse_iterator rend() const;

	void               clearDelta() const;
	const bool         isDirty() const;
	void               pack(ByteStream &target) const;
	void               packDelta(ByteStream &target) const;

	SetType const &    get() const;
	const size_t       size() const;
	bool               empty() const;
	bool               contains(ValueType const &t) const;
	const_iterator find(ValueType const &t) const;

	virtual void       clear();
	virtual const_iterator erase(ValueType const &t);
	virtual const_iterator erase(const_iterator &i);
	virtual void       insert(ValueType const &t);
	virtual void       unpack(ReadIterator &source);
	virtual void       unpackDelta(ReadIterator &source);

	static void        pack(ByteStream &target, std::set<ValueType> const &data);
	static void        unpack(ReadIterator &source, std::vector<Command> &data);
	static void        unpackDelta(ReadIterator &source, std::vector<Command> &data);

	void               setOnChanged(ObjectType *owner, void (ObjectType::*onChanged)());
	void               setOnErase(ObjectType *owner, void (ObjectType::*onErase)(ValueType const &));
	void               setOnInsert(ObjectType *owner, void (ObjectType::*onInsert)(ValueType const &));

private:
	void onChanged();
	void onErase(const ValueType &);
	void onInsert(const ValueType &);

private:
	SetType m_set;
	size_t m_baselineCommandCount;
	mutable std::vector<Command> m_commands;
	std::pair<ObjectType *, void (ObjectType::*)()> *m_onChangedCallback;
	std::pair<ObjectType *, void (ObjectType::*)(ValueType const &)> *m_onEraseCallback;
	std::pair<ObjectType *, void (ObjectType::*)(ValueType const &)> *m_onInsertCallback;

private:
	AutoDeltaSet(AutoDeltaSet const &);
};

//-----------------------------------------------------------------------

template<typename ValueType, typename ObjectType>
inline AutoDeltaSet<ValueType, ObjectType>::AutoDeltaSet() :
	AutoDeltaContainer(),
	m_set(),
	m_baselineCommandCount(0),
	m_commands(),
	m_onChangedCallback(0),
	m_onEraseCallback(0),
	m_onInsertCallback(0)
{
}

//-----------------------------------------------------------------------

template<typename ValueType, typename ObjectType>
inline AutoDeltaSet<ValueType, ObjectType>::~AutoDeltaSet()
{
	delete m_onChangedCallback;
	delete m_onEraseCallback;
	delete m_onInsertCallback;
}

//-----------------------------------------------------------------------

template<typename ValueType, typename ObjectType>
inline typename AutoDeltaSet<ValueType, ObjectType>::const_iterator AutoDeltaSet<ValueType, ObjectType>::begin() const
{
	return m_set.begin();
}

//-----------------------------------------------------------------------

template<typename ValueType, typename ObjectType>
inline void AutoDeltaSet<ValueType, ObjectType>::clear()
{
	if (!empty())
	{
		Command c;
		c.cmd = Command::CLEAR;
		m_commands.push_back(c);
		++m_baselineCommandCount;
		for (typename SetType::iterator i = m_set.begin(); i != m_set.end(); ++i)
			onErase(*i);
		m_set.clear();
		touch();
		onChanged();
	}
}

//-----------------------------------------------------------------------

template<typename ValueType, typename ObjectType>
inline void AutoDeltaSet<ValueType, ObjectType>::clearDelta()  const
{
	m_commands.clear();
}

//-----------------------------------------------------------------------

template<typename ValueType, typename ObjectType>
inline typename AutoDeltaSet<ValueType, ObjectType>::const_iterator AutoDeltaSet<ValueType, ObjectType>::end() const
{
	return m_set.end();
}

//-----------------------------------------------------------------------

template<typename ValueType, typename ObjectType>
inline typename AutoDeltaSet<ValueType, ObjectType>::const_iterator AutoDeltaSet<ValueType, ObjectType>::erase(ValueType const &value)
{
	typename SetType::const_iterator i(find(value));

	return erase(i);
}

//-----------------------------------------------------------------------

template<typename ValueType, typename ObjectType>
inline typename AutoDeltaSet<ValueType, ObjectType>::const_iterator AutoDeltaSet<ValueType, ObjectType>::erase(const_iterator &i)
{
	if (i != m_set.end())
	{
		Command c;
		c.cmd = Command::ERASE;
		c.value = *i;
		m_commands.push_back(c);
		++m_baselineCommandCount;
		
		// hack to convert from const_iterator to iterator as required by std libs
		typename SetType::iterator tmp(m_set.begin());
        	std::advance(tmp, std::distance<const_iterator>(tmp, i));
        	i++;
        	m_set.erase(tmp);

		touch();
		onErase(c.value);
		onChanged();
	}

	return i;
}

//-----------------------------------------------------------------------

template<typename ValueType, typename ObjectType>
inline bool AutoDeltaSet<ValueType, ObjectType>::contains(ValueType const &value) const
{
	return m_set.find(value) != m_set.end();
}

//-----------------------------------------------------------------------

template<typename ValueType, typename ObjectType>
inline typename AutoDeltaSet<ValueType, ObjectType>::const_iterator AutoDeltaSet<ValueType, ObjectType>::find(ValueType const &value) const
{
	return m_set.find(value);
}

//-----------------------------------------------------------------------

template<typename ValueType, typename ObjectType>
inline typename AutoDeltaSet<ValueType, ObjectType>::SetType const &AutoDeltaSet<ValueType, ObjectType>::get() const
{
	return m_set;
}

//-----------------------------------------------------------------------

template<typename ValueType, typename ObjectType>
inline void AutoDeltaSet<ValueType, ObjectType>::insert(ValueType const &value)
{
	if (m_set.insert(value).second)
	{
		Command c;
		c.cmd = Command::INSERT;
		c.value = value;
		m_commands.push_back(c);
		++m_baselineCommandCount;
		touch();
		onInsert(value);
		onChanged();
	}
}

//-----------------------------------------------------------------------

template<typename ValueType, typename ObjectType>
inline const bool AutoDeltaSet<ValueType, ObjectType>::isDirty() const
{
	return m_commands.size() > 0;
}

//-----------------------------------------------------------------------

template<typename ValueType, typename ObjectType>
inline void AutoDeltaSet<ValueType, ObjectType>::onChanged()
{
	if (m_onChangedCallback)
	{
		if (m_onChangedCallback->first)
		{
			ObjectType &owner = *m_onChangedCallback->first;
			void (ObjectType::*cb)() = m_onChangedCallback->second;
			(owner.*cb)();
		}
	}
}

//-----------------------------------------------------------------------

template<typename ValueType, typename ObjectType>
inline void AutoDeltaSet<ValueType, ObjectType>::onErase(ValueType const &value)
{
	if (m_onEraseCallback)
	{
		if (m_onEraseCallback->first)
		{
			ObjectType &owner = *m_onEraseCallback->first;
			void (ObjectType::*cb)(ValueType const &) = m_onEraseCallback->second;
			(owner.*cb)(value);
		}
	}
}

//-----------------------------------------------------------------------

template<typename ValueType, typename ObjectType>
inline void AutoDeltaSet<ValueType, ObjectType>::onInsert(ValueType const &value)
{
	if (m_onInsertCallback)
	{
		if (m_onInsertCallback->first)
		{
			ObjectType &owner = *m_onInsertCallback->first;
			void (ObjectType::*cb)(ValueType const &) = m_onInsertCallback->second;
			(owner.*cb)(value);
		}
	}
}

//-----------------------------------------------------------------------

template<typename ValueType, typename ObjectType>
inline void AutoDeltaSet<ValueType, ObjectType>::pack(ByteStream &target) const
{
	Archive::put(target, m_set.size());
	Archive::put(target, m_baselineCommandCount);
	for (typename SetType::const_iterator i = m_set.begin(); i != m_set.end(); ++i)
		put(target, *i);
}

// ----------------------------------------------------------------------

template<typename ValueType, typename ObjectType>
inline void AutoDeltaSet<ValueType, ObjectType>::pack(ByteStream &target, std::set<ValueType> const &data)
{
	Archive::put(target, data.size());
	Archive::put(target, static_cast<size_t>(0)); // baselineCommandCount
	for (typename std::set<ValueType>::const_iterator i = data.begin(); i != data.end(); ++i)
		put(target, *i);
}

//-----------------------------------------------------------------------

template<typename ValueType, typename ObjectType>
inline void AutoDeltaSet<ValueType, ObjectType>::packDelta(ByteStream &target) const
{
	Archive::put(target, m_commands.size());
	Archive::put(target, m_baselineCommandCount);
	for (typename std::vector<Command>::iterator i = m_commands.begin(); i != m_commands.end(); ++i)
	{
		Command const &c = *i;
		Archive::put(target, c.cmd);
		switch (c.cmd)
		{
		case Command::ERASE:
		case Command::INSERT:
			put(target, c.value);
			break;
		case Command::CLEAR:
			break;
		}
	}
	clearDelta();
}

//-----------------------------------------------------------------------

template<typename ValueType, typename ObjectType>
inline typename AutoDeltaSet<ValueType, ObjectType>::const_reverse_iterator AutoDeltaSet<ValueType, ObjectType>::rbegin() const
{
	return m_set.rbegin();
}

//-----------------------------------------------------------------------

template<typename ValueType, typename ObjectType>
inline typename AutoDeltaSet<ValueType, ObjectType>::const_reverse_iterator AutoDeltaSet<ValueType, ObjectType>::rend() const
{
	return m_set.rend();
}

//-----------------------------------------------------------------------

template<typename ValueType, typename ObjectType>
inline void AutoDeltaSet<ValueType, ObjectType>::setOnChanged(ObjectType *owner, void (ObjectType::*cb)())
{
	delete m_onChangedCallback;
	m_onChangedCallback = new std::pair<ObjectType *, void (ObjectType::*)()>;
	m_onChangedCallback->first = owner;
	m_onChangedCallback->second = cb;
}

//-----------------------------------------------------------------------

template<typename ValueType, typename ObjectType>
inline void AutoDeltaSet<ValueType, ObjectType>::setOnErase(ObjectType *owner, void (ObjectType::*cb)(ValueType const &))
{
	delete m_onEraseCallback;
	m_onEraseCallback = new std::pair<ObjectType *, void (ObjectType::*)(ValueType const &)>;
	m_onEraseCallback->first = owner;
	m_onEraseCallback->second = cb;
}

//-----------------------------------------------------------------------

template<typename ValueType, typename ObjectType>
inline void AutoDeltaSet<ValueType, ObjectType>::setOnInsert(ObjectType *owner, void (ObjectType::*cb)(ValueType const &))
{
	delete m_onInsertCallback;
	m_onInsertCallback = new std::pair<ObjectType *, void (ObjectType::*)(ValueType const &)>;
	m_onInsertCallback->first = owner;
	m_onInsertCallback->second = cb;
}

//-----------------------------------------------------------------------

template<typename ValueType, typename ObjectType>
inline bool AutoDeltaSet<ValueType, ObjectType>::empty() const
{
	return m_set.empty();
}

//-----------------------------------------------------------------------

template<typename ValueType, typename ObjectType>
inline const size_t AutoDeltaSet<ValueType, ObjectType>::size() const
{
	return m_set.size();
}

//-----------------------------------------------------------------------

template<typename ValueType, typename ObjectType>
inline void AutoDeltaSet<ValueType, ObjectType>::unpack(ReadIterator &source)
{
	m_set.clear();
	clearDelta();

	size_t commandCount;
	ValueType value;

	Archive::get(source, commandCount);
	Archive::get(source, m_baselineCommandCount);
	using Archive::get;
	for (size_t i = 0; i < commandCount; ++i)
	{
		get(source, value);
		m_set.insert(value);
	}

	onChanged();
}

// ----------------------------------------------------------------------

template<typename ValueType, typename ObjectType>
inline void AutoDeltaSet<ValueType, ObjectType>::unpack(ReadIterator &source, std::vector<Command> &data)
{
	Command c;
	size_t commandCount, targetBaselineCommandCount;

	Archive::get(source, commandCount);
	Archive::get(source, targetBaselineCommandCount);

	c.cmd = Command::INSERT;
	for (size_t i = 0; i < commandCount; ++i)
	{
		Archive::get(source,c.value);
		data.push_back(c);
	}
}

// ----------------------------------------------------------------------

template<typename ValueType, typename ObjectType>
inline void AutoDeltaSet<ValueType, ObjectType>::unpackDelta(ReadIterator &source, std::vector<Command> &data)
{
	Command c;
	size_t commandCount, targetBaselineCommandCount;

	Archive::get(source, commandCount);
	Archive::get(source, targetBaselineCommandCount);

	for (size_t i = 0 ; i < commandCount; ++i)
	{
		Archive::get(source, c.cmd);
		switch (c.cmd)
		{
			case Command::ERASE:
			case Command::INSERT:
				Archive::get(source, c.value);
				break;
			case Command::CLEAR:
				break;
			default:
				assert(false);
				break;
		}
		data.push_back(c);
	}
}

//-----------------------------------------------------------------------

template<typename ValueType, typename ObjectType>
inline void AutoDeltaSet<ValueType, ObjectType>::unpackDelta(ReadIterator &source)
{
	using Archive::get;
	Command c;
	size_t skipCount, commandCount, targetBaselineCommandCount;

	Archive::get(source, commandCount);
	Archive::get(source, targetBaselineCommandCount);

	// if (commandCount+baselineCommandCount) < targetBaselineCommandCount, it
	// means that we have missed some changes and are behind; when this happens,
	// catch up by applying all the deltas that came in, and set
	// baselineCommandCount to targetBaselineCommandCount
	if ((commandCount+m_baselineCommandCount) > targetBaselineCommandCount)
		skipCount = commandCount+m_baselineCommandCount-targetBaselineCommandCount;
	else
		skipCount = 0;

	if (skipCount > commandCount)
		skipCount = commandCount;

	size_t i = 0;
	for ( ; i < skipCount; ++i)
	{
		Archive::get(source, c.cmd);
		if (c.cmd != Command::CLEAR)
			get(source, c.value);
	}
	for ( ; i < commandCount; ++i)
	{
		Archive::get(source, c.cmd);
		switch (c.cmd)
		{
		case Command::ERASE:
			get(source, c.value);
			AutoDeltaSet::erase(c.value);
			break;
		case Command::INSERT:
			get(source, c.value);
			AutoDeltaSet::insert(c.value);
			break;
		case Command::CLEAR:
			AutoDeltaSet::clear();
			break;
		}
	}

	// if we are behind, catch up
	if (m_baselineCommandCount < targetBaselineCommandCount)
		m_baselineCommandCount = targetBaselineCommandCount;
}

//---------------------------------------------------------------------

}//namespace Archive

// ======================================================================

#endif	// _INCLUDED__AutoDeltaSet_H
