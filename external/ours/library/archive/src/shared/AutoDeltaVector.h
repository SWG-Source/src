#ifndef	_INCLUDED_AutoDeltaVector_H
#define	_INCLUDED_AutoDeltaVector_H

//-----------------------------------------------------------------------

#include "AutoDeltaByteStream.h"

//-----------------------------------------------------------------------

namespace Archive {

//-----------------------------------------------------------------------

template<typename ValueType, typename ObjectType=DefaultObjectType>
class AutoDeltaVector : public AutoDeltaContainer
{
	typedef typename std::vector<ValueType>::size_type size_type;

public:
	typedef typename std::vector<ValueType>::const_iterator const_iterator;
	typedef typename std::vector<ValueType>::const_reverse_iterator const_reverse_iterator;
	typedef std::vector<ValueType> VectorType;

public:
	struct Command
	{
		enum
		{
			ERASE,
			INSERT,
			SET,
			SETALL,
			CLEAR
		};
		unsigned char cmd;
		unsigned short int  index;
		ValueType     value;
	};

public:
	AutoDeltaVector();
	explicit AutoDeltaVector(size_t initialSize);
	~AutoDeltaVector();

	const ValueType &  back          () const;
	const_iterator     begin         () const;
	void               clearDelta    () const;
	const_iterator     end           () const;
	const int          find          (const ValueType & t) const;
	const ValueType &  front         () const;
	const VectorType & get           () const;
	const ValueType &  get           (const unsigned int element) const;
	const bool         isDirty       () const;
	void               pack          (ByteStream & target) const;
	void               packDelta     (ByteStream & target) const;
	void               reserve       (size_type n);
	void               resize        (size_type n, ValueType x = ValueType());

	virtual void       clear         ();
	virtual void       erase         (const unsigned int element);
	virtual void       pop_back      ();
	virtual void       push_back     (const ValueType & t);
	virtual void       insert        (const unsigned int before, const ValueType & newValue);
	virtual void       set           (const unsigned int element, const ValueType & newValue);
	virtual void       set           (const VectorType & newValue);
	virtual void       unpack        (ReadIterator & source);
	virtual void       unpackDelta   (ReadIterator & source);
	static void        pack          (ByteStream & target, const std::vector<ValueType> & data);
	static void        unpack        (ReadIterator & source, std::vector<Command> & data);
	static void        unpackDelta   (ReadIterator & source, std::vector<Command> & data);

	void               setOnChanged  (ObjectType * owner, void (ObjectType::*onChanged)());
	void               setOnErase    (ObjectType * owner, void (ObjectType::*onErase)(const unsigned int, const ValueType &));
	void               setOnInsert   (ObjectType * owner, void (ObjectType::*onInsert)(const unsigned int, const ValueType &));
	void               setOnSet      (ObjectType * owner, void (ObjectType::*onSet)(const unsigned int, const ValueType &, const ValueType &));

	const size_t       size          () const;
	bool               empty         () const;

	const ValueType &  operator[]    (const unsigned int element) const;

	const_reverse_iterator      rbegin() const;
	const_reverse_iterator      rend() const;

private:
	void packCommand(const Command & command, ByteStream & target) const;
	void onChanged();
	void onErase(const unsigned int, const ValueType &);
	void onInsert(const unsigned int, const ValueType &);
	void onSet(const unsigned int, const ValueType &, const ValueType &);

private:
	std::vector<ValueType>	v;
	size_t baselineCommandCount;
	mutable std::vector<Command>    commands;
	std::pair<ObjectType *, void (ObjectType::*)()> * onChangedCallback;
	std::pair<ObjectType *, void (ObjectType::*)(const unsigned int, const ValueType &)> * onEraseCallback;
	std::pair<ObjectType *, void (ObjectType::*)(const unsigned int, const ValueType &)> * onInsertCallback;
	std::pair<ObjectType *, void (ObjectType::*)(const unsigned int, const ValueType &, const ValueType &)> * onSetCallback;

private:
	AutoDeltaVector(const AutoDeltaVector &);
};

//-----------------------------------------------------------------------

template<typename ValueType, typename ObjectType>
inline AutoDeltaVector<ValueType, ObjectType>::AutoDeltaVector() :
AutoDeltaContainer(),
v(),
baselineCommandCount(0),
commands(),
onChangedCallback(0),
onEraseCallback(0),
onInsertCallback(0),
onSetCallback(0)
{
}

//-----------------------------------------------------------------------

template<typename ValueType, typename ObjectType>
inline AutoDeltaVector<ValueType, ObjectType>::AutoDeltaVector(size_t initialSize) :
AutoDeltaContainer(),
v(initialSize),
baselineCommandCount(0),
commands(),
onChangedCallback(0),
onEraseCallback(0),
onInsertCallback(0),
onSetCallback(0)
{
}

//-----------------------------------------------------------------------

template<typename ValueType, typename ObjectType>
inline AutoDeltaVector<ValueType, ObjectType>::~AutoDeltaVector()
{
	delete onChangedCallback;
	delete onEraseCallback;
	delete onInsertCallback;
	delete onSetCallback;
}

//-----------------------------------------------------------------------

template<typename ValueType, typename ObjectType>
inline typename AutoDeltaVector<ValueType, ObjectType>::const_iterator AutoDeltaVector<ValueType, ObjectType>::begin() const
{
	return v.begin();
}

//-----------------------------------------------------------------------

/**
* @todo this is very inefficient
*/
template<typename ValueType, typename ObjectType>
inline void AutoDeltaVector<ValueType, ObjectType>::clear()
{
	if (!empty())
	{
		Command c;
		c.cmd = Command::CLEAR;
		commands.push_back(c);
		++baselineCommandCount;
		for (unsigned int i = 0; i < v.size(); ++i)
			onErase(i, v[i]);
		v.clear();
		touch();
		onChanged();
	}
}

//-----------------------------------------------------------------------

template<typename ValueType, typename ObjectType>
inline const ValueType & AutoDeltaVector<ValueType, ObjectType>::back() const
{
	return v.back();
}

//-----------------------------------------------------------------------

template<typename ValueType, typename ObjectType>
inline void AutoDeltaVector<ValueType, ObjectType>::clearDelta()  const
{
	commands.clear();
}

//-----------------------------------------------------------------------

template<typename ValueType, typename ObjectType>
inline typename AutoDeltaVector<ValueType, ObjectType>::const_iterator AutoDeltaVector<ValueType, ObjectType>::end() const
{
	return v.end();
}

//-----------------------------------------------------------------------

template<typename ValueType, typename ObjectType>
inline void AutoDeltaVector<ValueType, ObjectType>::erase(const unsigned int element)
{
	if(element < size())
	{
		Command c;
		c.cmd = Command::ERASE;

		//-- cast to shorts to reduce net traffic. If we are synchronizing
		// vectors with more than 65,535 elements, we have some serious
		// design issues
		c.index = static_cast<unsigned short>(element);
		//--

		commands.push_back(c);
		++baselineCommandCount;
		typename std::vector<ValueType>::iterator i = v.begin();
		std::advance(i, element);
		ValueType old = (*i);
		v.erase(i);
		touch();
		onErase(element, old);
		onChanged();
	}
}
//-----------------------------------------------------------------------
template<typename ValueType, typename ObjectType>
inline const int AutoDeltaVector<ValueType, ObjectType>::find(const ValueType & item) const
{
	int count = 0;
	for (typename std::vector<ValueType>::const_iterator i = v.begin(); i != v.end(); ++i)
	{
		if (item == (*i))
		{
			return count;
		}
		++count;
	}
	return -1;
}

//-----------------------------------------------------------------------

template<typename ValueType, typename ObjectType>
inline const ValueType & AutoDeltaVector<ValueType, ObjectType>::front() const
{
	assert (!v.empty ());
	return v.front();
}

//-----------------------------------------------------------------------

template<typename ValueType, typename ObjectType>
inline const ValueType & AutoDeltaVector<ValueType, ObjectType>::get(const unsigned int element) const
{
	assert (v.size () > element);
	return v[element];
}

//----------------------------------------------------------------------

template<typename ValueType, typename ObjectType>
inline const typename AutoDeltaVector<ValueType, ObjectType>::VectorType & AutoDeltaVector<ValueType, ObjectType>::get() const
{
	return v;
}

//-----------------------------------------------------------------------

template<typename ValueType, typename ObjectType>
inline void AutoDeltaVector<ValueType, ObjectType>::insert(const unsigned int before, const ValueType & newValue)
{
	Command c;
	c.cmd = Command::INSERT;
	c.index = static_cast<unsigned short>(before);
	c.value = newValue;
	commands.push_back(c);
	++baselineCommandCount;

	if (v.size() < before)
		v.resize(before);

	typename std::vector<ValueType>::iterator i = v.begin();
	std::advance(i, before);
	v.insert(i, newValue);
	touch();
	onInsert(before, newValue);
	onChanged();
}

//-----------------------------------------------------------------------

template<typename ValueType, typename ObjectType>
inline const bool AutoDeltaVector<ValueType, ObjectType>::isDirty() const
{
	return commands.size() > 0;
}

//-----------------------------------------------------------------------

template<typename ValueType, typename ObjectType>
inline void AutoDeltaVector<ValueType, ObjectType>::onChanged()
{
	if(onChangedCallback)
	{
		if(onChangedCallback->first)
		{
			ObjectType & owner = *onChangedCallback->first;
			void (ObjectType::*cb)() = onChangedCallback->second;
			(owner.*cb)();
		}
	}
}

//-----------------------------------------------------------------------

template<typename ValueType, typename ObjectType>
inline void AutoDeltaVector<ValueType, ObjectType>::onErase(const unsigned int element, const ValueType & oldValue)
{
	if(onEraseCallback)
	{
		if(onEraseCallback->first)
		{
			ObjectType & owner = *onEraseCallback->first;
			void (ObjectType::*cb)(const unsigned int, const ValueType &) = onEraseCallback->second;
			(owner.*cb)(element, oldValue);
		}
	}
}

//-----------------------------------------------------------------------

template<typename ValueType, typename ObjectType>
inline void AutoDeltaVector<ValueType, ObjectType>::onInsert(const unsigned int element, const ValueType & newValue)
{
	if(onInsertCallback)
	{
		if(onInsertCallback->first)
		{
			ObjectType & owner = *onInsertCallback->first;
			void (ObjectType::*cb)(const unsigned int, const ValueType &) = onInsertCallback->second;
			(owner.*cb)(element, newValue);
		}
	}
}

//-----------------------------------------------------------------------

template<typename ValueType, typename ObjectType>
inline void AutoDeltaVector<ValueType, ObjectType>::onSet(const unsigned int element, const ValueType & oldValue, const ValueType & newValue)
{
	if(onSetCallback)
	{
		if(onSetCallback->first)
		{
			ObjectType & owner = *onSetCallback->first;
			void (ObjectType::*cb)(const unsigned int, const ValueType &, const ValueType &) = onSetCallback->second;
			(owner.*cb)(element, oldValue, newValue);
		}
	}
}

//-----------------------------------------------------------------------

template<typename ValueType, typename ObjectType>
inline void AutoDeltaVector<ValueType, ObjectType>::pack(ByteStream & target) const
{
	Archive::put(target, v.size());
	Archive::put(target, baselineCommandCount);
	typename std::vector<ValueType>::const_iterator i;
	for (i = v.begin(); i != v.end(); ++i)
	{
		put(target, (*i));
	}
}

// ----------------------------------------------------------------------

template<typename ValueType, typename ObjectType>
inline void AutoDeltaVector<ValueType, ObjectType>::pack(ByteStream & target, const std::vector<ValueType> & data)
{
	Archive::put(target, data.size());
	Archive::put(target, static_cast<size_t>(0)); // baselineCommandCount
	typename std::vector<ValueType>::const_iterator i;
	for (i = data.begin(); i != data.end(); ++i)
	{
		put(target, *i);
	}
}

//-----------------------------------------------------------------------

template<typename ValueType, typename ObjectType>
inline void AutoDeltaVector<ValueType, ObjectType>::packDelta(ByteStream & target) const
{
	Archive::put(target, commands.size());
	Archive::put(target, baselineCommandCount);
	typename std::vector<Command>::iterator i;

	for (i = commands.begin(); i != commands.end(); ++i)
	{
		const Command & c = (*i);
		Archive::put(target, c.cmd);
		switch(c.cmd)
		{
		case Command::ERASE:
			Archive::put(target, c.index);
			break;
		case Command::INSERT:
			Archive::put(target, c.index);
			put(target, c.value);
			break;
		case Command::SET:
			Archive::put(target, c.index);
			put(target, c.value);
			break;
		case Command::SETALL:
			{
				Archive::put(target, c.index); // size
				for (unsigned int j = 0; j < c.index; ++j)
				{
					++i;
					put(target, (*i).value);
				}
			}
			break;
		case Command::CLEAR:
			break;
		}
	}
	clearDelta();
}

//-----------------------------------------------------------------------

template<typename ValueType, typename ObjectType>
inline void AutoDeltaVector<ValueType, ObjectType>::pop_back()
{
	if (v.size() > 0)
		AutoDeltaVector::erase(v.size() - 1);
}

//-----------------------------------------------------------------------

template<typename ValueType, typename ObjectType>
inline void AutoDeltaVector<ValueType, ObjectType>::push_back(const ValueType & t)
{
	AutoDeltaVector::insert(v.size(), t);
}

//-----------------------------------------------------------------------

template<typename ValueType, typename ObjectType>
inline typename AutoDeltaVector<ValueType, ObjectType>::const_reverse_iterator AutoDeltaVector<ValueType, ObjectType>::rbegin() const
{
	return v.rbegin();
}

//-----------------------------------------------------------------------

template<typename ValueType, typename ObjectType>
inline typename AutoDeltaVector<ValueType, ObjectType>::const_reverse_iterator AutoDeltaVector<ValueType, ObjectType>::rend() const
{
	return v.rend();
}

//-----------------------------------------------------------------------

template<typename ValueType, typename ObjectType>
inline void AutoDeltaVector<ValueType, ObjectType>::reserve(size_type n)
{
	v.reserve(n);
}

//-----------------------------------------------------------------------

template<typename ValueType, typename ObjectType>
inline void AutoDeltaVector<ValueType, ObjectType>::resize(size_type n, ValueType x)
{
	size_type size = v.size();
	if (size < n)
	{
		// expand the vector
		v.resize(n);
		for (size_t i = size; i < n; ++i)
			set(i, x);
	}
	else if (size > n)
	{
		// contract the vector
		for (size_t i = size - 1; i >= n; --i)
		{
			erase(i);

			//-- must do this since our counter is unsigned
			if (i == 0)
				break;
		}
	}
}

//-----------------------------------------------------------------------

template<typename ValueType, typename ObjectType>
inline void AutoDeltaVector<ValueType, ObjectType>::set(const unsigned int element, const ValueType & newValue)
{
	if (element < v.size () && v[element] == newValue)
		return;

	Command c;
	c.cmd = Command::SET;
	c.index = static_cast<unsigned short>(element);
	c.value = newValue;
	commands.push_back(c);
	++baselineCommandCount;

	if(element >= v.size())
		v.resize(element + 1);

	ValueType old = v[element];

	v[element] = newValue;
	touch();
	onSet(element, old, newValue);
	onChanged();
}

//-----------------------------------------------------------------------

template<typename ValueType, typename ObjectType>
inline void AutoDeltaVector<ValueType, ObjectType>::set(const VectorType & newValue)
{
	Command c;
	c.cmd = Command::SETALL;
	c.index = static_cast<unsigned short>(newValue.size());
	commands.push_back(c);
	++baselineCommandCount;

	v = newValue;

	for (unsigned int i = 0; i < newValue.size(); ++i)
	{
		c.cmd = Command::SET;
		c.index = static_cast<unsigned short>(i);
		c.value = v[i];
		commands.push_back(c);
		++baselineCommandCount;
	}

	touch();
	onChanged();
}

//-----------------------------------------------------------------------

template<typename ValueType, typename ObjectType>
inline void AutoDeltaVector<ValueType, ObjectType>::setOnChanged(ObjectType * owner, void (ObjectType::*cb)())
{
	delete onChangedCallback;
	onChangedCallback = new std::pair<ObjectType *, void (ObjectType::*)()>;
	onChangedCallback->first = owner;
	onChangedCallback->second = cb;
}

//-----------------------------------------------------------------------

template<typename ValueType, typename ObjectType>
inline void AutoDeltaVector<ValueType, ObjectType>::setOnErase(ObjectType * owner, void (ObjectType::*cb)(const unsigned int, const ValueType &))
{
	delete onEraseCallback;
	onEraseCallback = new std::pair<ObjectType *, void (ObjectType::*)(const unsigned int, const ValueType &)>;
	onEraseCallback->first = owner;
	onEraseCallback->second = cb;
}

//-----------------------------------------------------------------------

template<typename ValueType, typename ObjectType>
inline void AutoDeltaVector<ValueType, ObjectType>::setOnInsert(ObjectType * owner, void (ObjectType::*cb)(const unsigned int, const ValueType &))
{
	delete onInsertCallback;
	onInsertCallback = new std::pair<ObjectType *, void (ObjectType::*)(const unsigned int, const ValueType &)>;
	onInsertCallback->first = owner;
	onInsertCallback->second = cb;
}

//-----------------------------------------------------------------------

template<typename ValueType, typename ObjectType>
inline void AutoDeltaVector<ValueType, ObjectType>::setOnSet(ObjectType * owner, void (ObjectType::*cb)(const unsigned int, const ValueType &, const ValueType &))
{
	delete onSetCallback;
	onSetCallback = new std::pair<ObjectType *, void (ObjectType::*)(const unsigned int, const ValueType &, const ValueType &)>;
	onSetCallback->first = owner;
	onSetCallback->second = cb;
}

//-----------------------------------------------------------------------

template<typename ValueType, typename ObjectType>
inline bool AutoDeltaVector<ValueType, ObjectType>::empty() const
{
	return v.empty();
}

//-----------------------------------------------------------------------

template<typename ValueType, typename ObjectType>
inline const size_t AutoDeltaVector<ValueType, ObjectType>::size() const
{
	return v.size();
}

//-----------------------------------------------------------------------

template<typename ValueType, typename ObjectType>
inline void AutoDeltaVector<ValueType, ObjectType>::unpack(ReadIterator & source)
{
	using Archive::get;
	// unpacking the whole kazaba
	v.clear();
	clearDelta();

	size_t commandCount;
	ValueType value;

	Archive::get(source, commandCount);
	Archive::get(source, baselineCommandCount);

	for (size_t i = 0; i < commandCount; ++i)
	{
		get(source, value);
		v.push_back(value);
	}

	onChanged();
}

// ----------------------------------------------------------------------

template<typename ValueType, typename ObjectType>
inline void AutoDeltaVector<ValueType, ObjectType>::unpack(ReadIterator & source, std::vector<Command> & data)
{
	using Archive::get;
	// unpacking the whole kazaba
	size_t commandCount;
	size_t bcc;
	Command c;

	Archive::get(source, commandCount);
	Archive::get(source, bcc);

	for (size_t i = 0; i < commandCount; ++i)
	{
		get(source,c.value);
		c.index = static_cast<unsigned short int>(i);
		c.cmd = Command::INSERT;

		data.push_back(c);
	}
}

// ----------------------------------------------------------------------

template<typename ValueType, typename ObjectType>
inline void AutoDeltaVector<ValueType, ObjectType>::unpackDelta(ReadIterator & source, std::vector<Command> & data)
{
	using Archive::get;
	Command c;
	size_t commandCount, targetBaselineCommandCount;

	Archive::get(source, commandCount);
	Archive::get(source, targetBaselineCommandCount);

	for (size_t i=0 ; i < commandCount; ++i)
	{
		Archive::get(source, c.cmd);
		switch (c.cmd)
		{
			case Command::ERASE:
				Archive::get(source, c.index);
				break;
			case Command::INSERT:
				Archive::get(source, c.index);
				get(source, c.value);
				break;
			case Command::SET:
				Archive::get(source, c.index);
				get(source, c.value);
				break;
			case Command::SETALL: // SETALL is not supported by the static AutoDeltaVector unpack function, because there's no place to put the value
				assert(false);
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
inline void AutoDeltaVector<ValueType, ObjectType>::unpackDelta(ReadIterator & source)
{
	using Archive::get;
	Command c;
	size_t skipCount, commandCount, targetBaselineCommandCount;

	Archive::get(source, commandCount);
	Archive::get(source, targetBaselineCommandCount);

	skipCount = commandCount+baselineCommandCount-targetBaselineCommandCount;

	// note: SETALL commands come across with a command count of 1+newsize
	if (skipCount > commandCount)
		skipCount = commandCount;

	size_t i;
	for (i = 0; i < skipCount; ++i)
	{
		Archive::get(source, c.cmd);
		if (c.cmd != Command::CLEAR)
			Archive::get(source, c.index);
		if (c.cmd == Command::SETALL)
		{
			for (unsigned int j = 0; j < c.index; ++j)
			{
				++i;
				get(source, c.value);
			}
		}
		else if (c.cmd != Command::ERASE && c.cmd != Command::CLEAR)
			get(source, c.value);
	}
	for ( ; i < commandCount; ++i)
	{
		Archive::get(source, c.cmd);
		switch (c.cmd)
		{
		case Command::ERASE:
			{
				get(source, c.index);
				AutoDeltaVector::erase(c.index);
			}
			break;
		case Command::INSERT:
			{
				Archive::get(source, c.index);
				get(source, c.value);
				AutoDeltaVector::insert(c.index, c.value);
			}
			break;
		case Command::SET:
			{
				Archive::get(source, c.index);
				get(source, c.value);
				AutoDeltaVector::set(c.index, c.value);
			}
			break;
		case Command::SETALL:
			{
				Archive::get(source, c.index); // size
				VectorType tempVec;
				tempVec.reserve(c.index);
				ValueType value;
				for (unsigned int j = 0; j < c.index; ++j)
				{
					++i;
					get(source, value);
					tempVec.push_back(value);
				}
				AutoDeltaVector::set(tempVec);
			}
			break;
		case Command::CLEAR:
			AutoDeltaVector::clear();
			break;
		}
	}
}

//---------------------------------------------------------------------

template<typename ValueType, typename ObjectType>
inline const ValueType & AutoDeltaVector<ValueType, ObjectType>::operator[] (const unsigned int element) const
{
	return get(element);
}

//---------------------------------------------------------------------

}//namespace Archive

#endif	// _INCLUDED__AutoDeltaVector_H


