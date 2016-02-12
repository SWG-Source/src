#ifndef	_INCLUDED_AutoDeltaVariableCallback_H
#define	_INCLUDED_AutoDeltaVariableCallback_H


#include "AutoDeltaByteStream.h"

namespace Archive {

/**
	@brief an authoritative variable that provides notification when changed

	An AutoDeltaVariableCallback is an AutoDeltaVariable.  It extends the functionality
	of AutoDeltaVariable by providing an interface for getting callbacks when the value changes.

	To use the AutoDeltaVariableCallback, you must pass it a callback structure.  An example is provided here:

	struct ExampleDataCallback
	{
		void modified(TangibleObject & target, const std::string & value) const;
	};

	This simple structure defines only one function, which will be invoked by the AutoDeltaVariableCallback
	when the variable is changed by a set(), operator=, or unpack() method.

	Then to declare an AutoDeltaVariableCallback as a member of an example class you might:

	class MyExampleClass
	{
	private:
		struct ExampleDataCallback
		{
			//define modified to call MyExampleClass::dataModified(value)
			void modified(MyExampleClass & target, ValueType & value) const {target.dataModified(value);}
		};

		//Friend it so that outsiders don't screw with the callback function but so that it can use it.
		friend struct ExampleDataCallback;
		void          dataModified(ValueType & value);

		Archive::AutoDeltaVariableCallback<ValueType, ExampleDataCallback, MyExampleClass> m_data;		
	};

	In the constructor of MyExampleClass:
		m_data->setSourceObject(this);  //This registers the correct instance of the object with the variable
		addVariable(m_data);            //See AutoDeltaVariable for why this is done.

	@see AutoDeltaVariable
	@see AutoDeltaVariableBase

	@author Chris Mayer
*/

template<class ValueType, class Callback, class SourceObjectType>
class AutoDeltaVariableCallback : public AutoDeltaVariable<ValueType>
{
public:

	AutoDeltaVariableCallback();
	AutoDeltaVariableCallback(const AutoDeltaVariableCallback & source);
	explicit AutoDeltaVariableCallback(const ValueType & source);
	~AutoDeltaVariableCallback();

	AutoDeltaVariableCallback & operator = (const AutoDeltaVariableCallback & source);
	AutoDeltaVariableCallback & operator = (const ValueType & source);

	virtual void         set(const ValueType & source);
	void                 setSourceObject(SourceObjectType * source);

	virtual void         unpack(ReadIterator & source);
	virtual void         unpackDelta(ReadIterator & source);

private:
	Callback             callback;
	SourceObjectType*    sourceObject;
};

//----------------------------------------------------------------
/**
	@brief default AutoDeltaVariableCallback constructor

	@author Chris Mayer

	@see AutoDeltaVariable
*/
template<class ValueType, class Callback, class SourceObjectType>
inline AutoDeltaVariableCallback<ValueType, Callback, SourceObjectType>::AutoDeltaVariableCallback() :
AutoDeltaVariable<ValueType>(),
sourceObject(0)
{
}

//----------------------------------------------------------------
/**
	@brief default AutoDeltaVariableCallback copy constructor

	@author Chris Mayer

	@see AutoDeltaVariable
*/
template<class ValueType, class Callback, class SourceObjectType>
inline AutoDeltaVariableCallback<ValueType, Callback, SourceObjectType>::AutoDeltaVariableCallback(const AutoDeltaVariableCallback & source) :
AutoDeltaVariable<ValueType>(source),
sourceObject(0)
{
}

//----------------------------------------------------------------

/**
	@brief default AutoDeltaVariableCallback constructor given value

	@author Chris Mayer

	@see AutoDeltaVariable
*/
template<class ValueType, class Callback, class SourceObjectType>
inline AutoDeltaVariableCallback<ValueType, Callback, SourceObjectType>::AutoDeltaVariableCallback(const ValueType & source) : 
AutoDeltaVariable<ValueType>(source),
sourceObject(0)
{
}

//----------------------------------------------------------------
/**
	@brief default AutoDeltaVariableCallback destructor

	@author Chris Mayer

	@see AutoDeltaVariable
*/
template<class ValueType, class Callback, class SourceObjectType>
inline AutoDeltaVariableCallback<ValueType, Callback, SourceObjectType>::~AutoDeltaVariableCallback()
{
}
//----------------------------------------------------------------

template<class ValueType, class Callback, class SourceObjectType>
inline AutoDeltaVariableCallback<ValueType, Callback, SourceObjectType> & AutoDeltaVariableCallback<ValueType, Callback, SourceObjectType>::operator =(const AutoDeltaVariableCallback<ValueType, Callback, SourceObjectType> & rhs) 
{
	if (&rhs != this)
		set(rhs.get());
	return *this;
}

//-----------------------------------------------------------------------

template<class ValueType, class Callback, class SourceObjectType>
inline AutoDeltaVariableCallback<ValueType, Callback, SourceObjectType> & AutoDeltaVariableCallback<ValueType, Callback, SourceObjectType>::operator = (const ValueType & rhs)
{
	if (rhs != this->get())
		set(rhs);
	return *this;
}

//-----------------------------------------------------------------------
/**
	@brief setSourceObject registers the object that owns the variable so that we can provide callback notification.

	This function should be called in the owning classes constructor before it is used.
	Failure to call this function before the variable's first use will cause an assertion failure.

	@author Chris Mayer

	@see AutoDeltaVariable
*/
template<class ValueType, class Callback, class SourceObjectType>
inline void AutoDeltaVariableCallback<ValueType, Callback, SourceObjectType>::setSourceObject(SourceObjectType * source)
{
	sourceObject = source;
}

//-----------------------------------------------------------------------

/**
	@brief overload AutoDeltaVariable::set to provide for callback notification

	::set() will notifiy the callback object after setting the value by calling the base class ::set() function.

	@author Chris Mayer

	@see AutoDeltaVariable
*/
template<class ValueType, class Callback, class SourceObjectType>
inline void AutoDeltaVariableCallback<ValueType, Callback, SourceObjectType>::set(const ValueType & source)
{
	ValueType const tmp = this->get();
	AutoDeltaVariable<ValueType>::set(source);
	
	if (sourceObject != nullptr && tmp != source)
		callback.modified(*sourceObject, tmp, source, true);
}

//-----------------------------------------------------------------------
/**
	@brief overload AutoDeltaVariable::unpack to provide for callback notification

	::unpack() will notifiy the callback object after setting the value by calling the base class ::unpack() function.

	@author Chris Mayer

	@see AutoDeltaVariable
*/
template<class ValueType, class Callback, class SourceObjectType>
inline void AutoDeltaVariableCallback<ValueType, Callback, SourceObjectType>::unpack(ReadIterator & source)
{
	ValueType const before = this->get();

	AutoDeltaVariable<ValueType>::unpack(source);

	if (sourceObject)
	{
		ValueType const &after = this->get();
		if (before != after)
			callback.modified(*sourceObject, before, after, false);
	}
}

//-----------------------------------------------------------------------

template<class ValueType, class Callback, class SourceObjectType>
inline void AutoDeltaVariableCallback<ValueType, Callback, SourceObjectType>::unpackDelta(ReadIterator & source)
{
	ValueType const before = this->get();

	AutoDeltaVariable<ValueType>::unpackDelta(source);

	if (sourceObject)
	{
		ValueType const &after = this->get();
		if (before != after)
			callback.modified(*sourceObject, before, after, false);
	}
}

//-----------------------------------------------------------------------

}

#endif

