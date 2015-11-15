
#ifndef	_INCLUDED_AutoDeltaByteStream_H
#define	_INCLUDED_AutoDeltaByteStream_H

//-----------------------------------------------------------------------

#if WIN32
#pragma warning ( disable : 4786 )
#pragma warning ( disable : 4800 )

#endif

#include <cmath>
#include "AutoByteStream.h"
#include "ByteStream.h"
#include <set>
#include <vector>

namespace Archive {

class AutoDeltaVariableBase;

class OnDirtyCallbackBase
{
public:
	OnDirtyCallbackBase();
	virtual ~OnDirtyCallbackBase() = 0;
	virtual void onDirty() = 0;
private:
	OnDirtyCallbackBase(const OnDirtyCallbackBase &);
	OnDirtyCallbackBase & operator =  (const OnDirtyCallbackBase &);
};

template<typename ObjectType>
class OnDirtyCallback : public OnDirtyCallbackBase
{
public:
	OnDirtyCallback() :
		owner(0),
		callback(0)
	{
	}

	OnDirtyCallback(ObjectType &o, void (ObjectType::*onDirty)()) :
		owner(&o),
		callback(onDirty)
	{
	}

	virtual ~OnDirtyCallback()
	{
	}

	void set(ObjectType &o, void (ObjectType::*onDirty)())
	{
		owner = &o;
		callback = onDirty;
	}

	virtual void onDirty()
	{
		if (owner && callback)
			((*owner).*callback)();
	}

private:
	OnDirtyCallback(const OnDirtyCallback &);
	OnDirtyCallback &  operator =  (const OnDirtyCallback &);

private:
	ObjectType *owner;
	void (ObjectType::*callback)();
};

//-----------------------------------------------------------------------
/**
	@brief An ByteStream that knows when it's members have changed.

	The AutoDeltaByteStream employs the AutoDeltaVariableBase to
	detect changes in it's members. Any AutoDetlaVaraibleBase that
	changed since the last packDeltas() method will be placed in the
	ByteStream during the next packDeltas() operation.

	ByteStream data is packed as <varIndex><value> in memory, so
	only AutoDeltaByteStreams may interpret a delta package at run time.

	AutoDeltaByteStream objects can also deserialize from ByteStream and
	AutoByteStream objects using the unpack() method. To unpack a
	delta ByteStream, AutoDeltaByteStream provides unpackDeltas().

	AutoDeltaVariable objects, which are contained by a
	AutoDeltaByteStream, can set threshold values to trigger a
	change (e.g. a value representing heading may need to change
	by more than 3 degrees before the object is considered "changed").


	Example Usage:
	\code
	class MyAutoDeltaByteStream : public AutoDeltaByteStream
	{
	public:
		MyAutoDeltaByteStream();
		~MyAutoDeltaByteStream();

		const float getValue() const;
		void        setValue(const float value);

	private:
		AutoDeltaVariable<float>	value;
	};

	MyAutoDeltaByteStream::MyAutoDeltaByteStream() :
	value(0.0f)
	{
		addVariable(value);
		value.setDeltaSensitivity(3.0f);
		value.clearDelta();
	}

	const float MyAutoDeltaByteStream::getValue() const
	{
		return value;
	}

	void MyAutoDeltaByteStream::setValue(const float newValue)
	{
		value = newValue;
	}

	void foo()
	{
		MyAutoDeltaByteStream a;
		MyAutoDeltaByteStream b;

		a.setValue(2.0f);	// change of 2, not enough to trigger a delta
		b.setValue(1.0f);

		b.unpackDeltas(a.packDeltas());
		assert(b.getValue() == 1.0f);	// no deltas from (a)

		a.setValue(5.0f);	// large enough change
		b.unpackDeltas(a.packDeltas());
		assert(b.getValue() == a.getValue() && b.getValue() == 5.0f);
	}
	\endcode


*/
class AutoDeltaByteStream : public AutoByteStream
{
public:
	                            AutoDeltaByteStream  ();
	virtual                     ~AutoDeltaByteStream ();

	void                        addOnDirtyCallback     (OnDirtyCallbackBase * newCallback);
	virtual void                addVariable            (AutoDeltaVariableBase & var);
	virtual const unsigned int  getItemCount           () const;
	virtual void                packDeltas             (ByteStream & target) const;
	virtual void                unpackDeltas           (ReadIterator & source);
	virtual void                clearDeltas            () const;
	void                        removeOnDirtyCallback  ();

protected:
	friend class AutoDeltaVariableBase;
	void                        addToDirtyList     (AutoDeltaVariableBase * var);

private:
	// disable assignment and copy constructors
	AutoDeltaByteStream &         operator =         (const AutoDeltaByteStream & rhs);
	                            AutoDeltaByteStream  (const AutoDeltaByteStream & source);

private:
	mutable std::set<void *>        dirtyList; // contains AutoVariable's that insert themselves on change
	OnDirtyCallbackBase *           onDirtyCallback;
};

//-----------------------------------------------------------------------
/**
	@brief An abstract base class for AutoDeltaVariables

	The AutoDeltaVariableBase defines interfaces for
	managing how delta's between packDelta operations
	are detected.

	@see AutoDeltaByteStream
	@see AutoByteStream
	@see ByteStream
	@see AutoVariableBase

	@author Justin Randall
*/
class AutoDeltaVariableBase : public AutoVariableBase
{
public:
	AutoDeltaVariableBase();
	virtual ~AutoDeltaVariableBase() = 0;

	/** pure virtual */
	virtual void        clearDelta() const = 0;

	const unsigned short int getIndex() const;

	/** pure virtual */
	virtual const bool  isDirty() const = 0;

	void                touch();
protected:
	friend class AutoDeltaByteStream;
	AutoDeltaByteStream * getOwner();
	void                setIndex(const unsigned short int index);
	void                setOwner(AutoDeltaByteStream * owner);

	/** pure virtual */
	virtual void        pack(ByteStream & target) const = 0;

	/** pure virtual */
	virtual void        packDelta(ByteStream & target) const = 0;

	/** pure virtual */
	virtual void        unpackDelta(ReadIterator & source) = 0;
private:
	unsigned short int  index;
	AutoDeltaByteStream * owner;
};

//-----------------------------------------------------------------------
/**
	@brief Return the AutoByteStream::members index for this instance.

	A AutoDeltaVariableBase tracks its index in the members list
	of the ByteStream. This helper-value permits deserialization of the
	value to the appropriate member when the AutoDeltaByteStream invokes
	unpackDeltas(). AutoByteStream::packDeltas() prefixes each
	delta value with the index of the AutoDeltaVariable.
	<index><value>

	@return the index of the variable in AutoByteStream::members

	@see AutoByteStream
	@see AutoDeltaByteStream
	@see ByteStream
	@see AutoDeltaByteStream::packDeltas
	@see AutoDeltaByteStream::unpackDeltas

	@author Justin Randall
*/
inline const unsigned short int AutoDeltaVariableBase::getIndex() const
{
	return index;
}

//-----------------------------------------------------------------------
/**
	@brief Get a pointer to this variable's owner ByteStream

	As the AutoDeltaVariableBase is used in a non-const
	manner, it will insert itself into it's owner AutoDeltaByteStream::dirtyList
	set.

	When the AutoDeltaByteStream executes AutoDeltaByteStream::packDeltas(),
	it will iterate through this set to determine which
	variables may be dirty, invoke AutoDeltaVariableBase::isDirty() on
	each variable in the set, and if it is dirty, include the value
	in the ByteStream buffer.

	AutoDeltaVariableBase derived classes need to retrieve a
	reference to their owner ByteStreams to add them to the
	dirtyList set.

	@return a pointer to the owner AutoDeltaByteStream

	@see AutoDeltaByteStream
	@see AutoDeltaByteStream::packDeltas
	@see AutoDeltaByteStream::unpackDeltas
	@see AutoDeltaVariableBase::pack
	@see AutoDeltaVariableBase::unpack

	@author Justin Randall
*/
inline AutoDeltaByteStream * AutoDeltaVariableBase::getOwner()
{
	return owner;
}

//-----------------------------------------------------------------------
/**
	@brief Set the index in the AutoByteStream Members list

	This method is overridden so that the owner value may also
	be set.

	@param newIndex   An index into the AutoAchive::members list

	@see AutoVariableBase
	@see AutoAchive
	@see AutoDeltaByteStream
	@see ByteStream

	@author Justin Randall
*/
inline void AutoDeltaVariableBase::setIndex(const unsigned short int newIndex)
{
	index = newIndex;
}

//-----------------------------------------------------------------------
/**
	@brief set the AutoDeltaVariableBase::owner member to the address of
	some AutoDeltaByteStream instance.

	A AutoDeltaVariableBase or derived object must put itself on
	it's owner AutoDeltaByteStream::dirtyList

*/
inline void AutoDeltaVariableBase::setOwner(AutoDeltaByteStream * newOwner)
{
	owner = newOwner;
}

//-----------------------------------------------------------------------
/**
	@brief Put this variable on the owner ByteStream's dirty list

	@author Justin Randall
*/
inline void AutoDeltaVariableBase::touch()
{
	if(owner)
		owner->addToDirtyList(this);
}

//-----------------------------------------------------------------------
/**
	@brief an authoritative variable that knows when it has changed

	A AutoDeltaVariable is use much like a AutoVariable, but it also
	knows when it has changed. In addition to knowing when it has changed,
	it also may set some change threshold, so that minor variations
	in the value can be ignored by the application.

	Example:
	\code
	class MyAutoDeltaByteStream : public AutoDeltaByteStream
	{
	public:
		MyAutoDeltaByteStream();
		~MyAutoDeltaByteStream() {};

		void      setValue(const int value)
			{ i = value; };

		const int getValue() const
			{ return i; };

	private:
		AutoDeltaVariable<int>	i;
	};

	MyAutoDeltaByteStream::MyAutoDeltaByteStream()
	{
		addVariable(i);
		i.setDeltaSensitivity(5.0f);
	};

	void foo()
	{
		MyAutoDeltaByteStream a;
		MyAutoDeltaByteStream b;
		ByteStream             c;

		c = a.pack();    // packs all values (just a.i, really)

		b << c;	        // unpacks all values
		assert(a.getValue() == b.getValue());

		a.setValue(3);
		b.unpackDeltas(a.packDeltas());
		assert(a.getValue() != b.getValue());

		a.setValue(300);
		b.unpackDeltas(a.packDeltas());
		assert(a.getValue() == b.getValue());
	}
	\endcode

	@see AutoDeltaVariableBase
	@see AutoVariable
	@see AutoVariableBase
	@see AutoDeltaByteStream
	@see AutoByteStream
	@see ByteStream

	@author Justin Randall
*/
template<typename ValueType>
class AutoDeltaVariable : public AutoDeltaVariableBase
{
public:
	AutoDeltaVariable();
	AutoDeltaVariable(const AutoDeltaVariable & source);
	explicit AutoDeltaVariable(const ValueType & source);
	~AutoDeltaVariable();

	AutoDeltaVariable & operator = (const AutoDeltaVariable & source);
	AutoDeltaVariable & operator = (const ValueType & source);

	void                 clearDelta() const;
	const ValueType &    get() const;
	const bool           isDirty() const;
	void                 pack(ByteStream & target) const;
	void                 packDelta(ByteStream & target) const;
	virtual void         set(const ValueType & source);
	virtual void         unpack(ReadIterator & source);
	virtual void         unpackDelta(ReadIterator & source);
private:
	ValueType            currentValue;
	mutable ValueType    lastValue;
};

//-----------------------------------------------------------------------
/**
	@brief default AutoDeltaVariable constructor

	@author Justin Randall

	@see AutoVariable
	@see AutoVariableBase
	@see AutoDeltaVariableBase
	@see AutoByteStream
	@see ByteStream
*/
template<typename ValueType>
inline AutoDeltaVariable<ValueType>::AutoDeltaVariable() :
AutoDeltaVariableBase(),
currentValue(ValueType()),
lastValue(ValueType())
{
}

//-----------------------------------------------------------------------
/**
	@brief AutoDeltaVariable copy constructor

	This copy constructor will perform a deep copy of the current
	value from the source AutoDeltaVariable. It does not copy
	AutoDeltaVariableBase members such as the owner. The owner MUST
	be set explicitly.

	@author Justin Randall

	@see AutoVariable
	@see AutoVariableBase
	@see AutoDeltaVariableBase
	@see AutoByteStream
	@see ByteStream
*/
template<typename ValueType>
inline AutoDeltaVariable<ValueType>::AutoDeltaVariable(const AutoDeltaVariable & source) :
AutoDeltaVariableBase(),
currentValue(source.currentValue),
lastValue(source.currentValue)
{
}

//-----------------------------------------------------------------------
/**
	@brief AutoDeltaVariable copy constructor

	A deep copy of the source value is placed in the current value. All
	other members use default constructors in the initalizer list.

	\code
	MyAutoDeltaByteStream::MyAutoDeltaByteStream()
	{
		// set myAutoInt current value to 35
		myAutoInt(35);
		myAutoInt.setOwner(this);

		// copy myAutoInt
		myAutoInt2(myAutoInt);
		myAutoInt2.setOwner(this);

		// myAutoInt2 now has a current value of 35

	}
	\endcode
	@author Justin Randall

	@see AutoVariable
	@see AutoVariableBase
	@see AutoDeltaVariableBase
	@see AutoByteStream
	@see ByteStream
*/
template<typename ValueType>
inline AutoDeltaVariable<ValueType>::AutoDeltaVariable(const ValueType & source) :
AutoDeltaVariableBase(),
currentValue(source),
lastValue(source)
{
}

//-----------------------------------------------------------------------
/**
	@brief AutoDeltaVariable destructor

	Nothing special happens in this desctructor.

	@see AutoDeltaVariableBase
	@see AutoByteStream
	@see AutoDeltaByteStream
	@see ByteStream

	@author Justin Randall
*/
template<typename ValueType>
inline AutoDeltaVariable<ValueType>::~AutoDeltaVariable()
{
}

//-----------------------------------------------------------------------

template<typename ValueType>
inline AutoDeltaVariable<ValueType> & AutoDeltaVariable<ValueType>::operator =(const AutoDeltaVariable<ValueType> & rhs)
{
	if(&rhs != this)
		set(rhs.currentValue);
	return *this;
}

//-----------------------------------------------------------------------

template<typename ValueType>
inline AutoDeltaVariable<ValueType> & AutoDeltaVariable<ValueType>::operator =(const ValueType & rhs)
{
	if(rhs != currentValue)
		set(rhs);
	return *this;
}

//-----------------------------------------------------------------------
/**
	@brief assign the lastValue to the currentValue to avert isDirty()
	returning true.

	@author Justin Randall
*/
template<typename ValueType>
inline void AutoDeltaVariable<ValueType>::clearDelta() const
{
	lastValue = currentValue;
}

//---------------------------------------------------------------------
/**
	@brief return the current value of the AutoDeltaVariable

	@return the current value of the AutoDeltaVariable

	@see AutoDeltaVariableBase
	@see AutoVariable
	@see AutoDeltaByteStream
	@see AutoByteStream
	@see ByteStream

	@author Justin Randall
*/
template<typename ValueType>
inline const ValueType & AutoDeltaVariable<ValueType>::get() const
{
	return currentValue;
}

//-----------------------------------------------------------------------
/**
	@brief compare last and current values to be sure the value
	to determine if the value has changed.

	When a AutoDeltaVariable is used in a non-const manner, it is placed
	on the AutoDeltaByteStream::dirtyList until AutoDeltaByteStream::packDeltas()
	is invoked. All members on the list are checked to see if their value
	has changed enough to warrant a new delta ByteStream.

	Any change in values triggers isDirty() to return true.

	@return true if the variable has changed

	@see AutoDeltaVariable
	@see AutoDeltaVariableBase
	@see AutoDeltaByteStream
	@see AutoVariable
	@see AutoVariableBase
	@see AutoByteStream
	@see ByteStream

	@author Justin Randall
*/
template<typename ValueType>
inline const bool AutoDeltaVariable<ValueType>::isDirty() const
{
	return lastValue != currentValue;
}

//-----------------------------------------------------------------------
/**
	@brief pack the ValueType into the target ByteStream

	If the ByteStream understands how to pack the type, or if
	a derived class can break the value into it's integral components,
	then it is placed into the ByteStream.

	This can be invoked either by an ByteStream base class, or from
	AutoDeltaByteStream::packDeltas(). If packing delta values, the
	AutoDeltaByteStream first packs an index number into the ByteStream. In
	this case, the ByteStream can ONLY be unpacked via another
	AutoDeltaByteStream::unpackDeltas(), and the other instance must
	declare the same AutoVariableBase types in exactly the same order!

	@param target   A target ByteStream object receiving the value

	@see AutoDeltaVariableBase
	@see AutoVariableBase
	@see AutoDeltaByteStream
	@see AutoByteStream
	@see ByteStream

	@author Justin Randall
*/
template<typename ValueType>
inline void AutoDeltaVariable<ValueType>::pack(ByteStream & target) const
{
	put(target, currentValue);
}

//-----------------------------------------------------------------------

template<typename ValueType>
inline void AutoDeltaVariable<ValueType>::packDelta(ByteStream & target) const
{
	pack(target);
	clearDelta();
}

//---------------------------------------------------------------------
/**
	@brief Set the current value of this AutoDeltaVariable

	@see AutoDeltaVariableBase
	@see AutoDeltaByteStream
	@see AutoVariable
	@see AutoByteStream
	@see ByteStream

	@author Justin Randall
*/
template<typename ValueType>
inline void AutoDeltaVariable<ValueType>::set(const ValueType & source)
{
	if (currentValue != source)
	{
		currentValue = source;
		touch();
	}
}

//-----------------------------------------------------------------------
/**
	@brief retrieve a value from the source ByteStream

	Read the variable value from the source ByteStream at the current read
	position in the ByteStream. To work properly, the ByteStream read position
	MUST contain the value -- that is, variables must be retrieve in the
	order they were packed.

	AutoByteStream classes and their derivatives automatically pack and
	unpack data in the correct order. This protected method is visible
	only to ByteStreams.

	@param source   The source ByteStream containing the desired value at
	                its current read position

	@see AutoDeltaVariable
	@see AutoDeltaVariableBase
	@see AutoDeltaByteStream
	@see AutoVariableBase
	@see AutoByteStream
	@see ByteStream

	@author Justin Randall
*/
template<typename ValueType>
inline void AutoDeltaVariable<ValueType>::unpack(ReadIterator & source)
{
	using Archive::get;
	get(source, currentValue);
	clearDelta();
}

//-----------------------------------------------------------------------

template<typename ValueType>
inline void AutoDeltaVariable<ValueType>::unpackDelta(ReadIterator & source)
{
	using Archive::get;
	get(source, currentValue);
	touch();
}

//---------------------------------------------------------------------

struct DefaultObjectType {};

//-----------------------------------------------------------------------

static const unsigned char CONTAINER_SET = 0;
static const unsigned char CONTAINER_ERASE = 1;
/**
	@brief base container synchronization class

	@author Justin Randall
*/
class AutoDeltaContainer : public AutoDeltaVariableBase
{
public:
	AutoDeltaContainer();
	/** pure virtual */
	virtual ~AutoDeltaContainer() = 0;
	/** pure virtual */
	virtual const size_t size() const = 0;
};

//-----------------------------------------------------------------------
/**
	@brief ctor

	@author Justin Randall
*/
inline AutoDeltaContainer::AutoDeltaContainer() :
AutoDeltaVariableBase()
{
}

//-----------------------------------------------------------------------
/**
	@brief dtor

	@author Justin Randall
*/
inline AutoDeltaContainer::~AutoDeltaContainer()
{
}

//-----------------------------------------------------------------------

}//namespace Archive

#endif	// _INCLUDED__AutoDeltaByteStream_H
