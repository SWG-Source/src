#ifndef	_INCLUDED_AutoDeltaVariableObserver_H
#define	_INCLUDED_AutoDeltaVariableObserver_H


#include "AutoDeltaByteStream.h"
#include "AutoDeltaObserverOps.h"

namespace Archive {

/**
	@brief an authoritative variable that provides notification both prior to and after change
*/

template<class ValueType, typename Callback, class SourceObjectType>
class AutoDeltaVariableObserver : public AutoDeltaVariable<ValueType>
{
public:

	AutoDeltaVariableObserver();
	AutoDeltaVariableObserver(const AutoDeltaVariableObserver & source);
	explicit AutoDeltaVariableObserver(const ValueType & source);
	~AutoDeltaVariableObserver();

	AutoDeltaVariableObserver & operator = (const AutoDeltaVariableObserver & source);
	AutoDeltaVariableObserver & operator = (const ValueType & source);

	virtual void         set(const ValueType & source);
	void                 setSourceObject(SourceObjectType * source);

	virtual void         unpack(ReadIterator & source);
	virtual void         unpackDelta(ReadIterator & source);

private:
	SourceObjectType*    sourceObject;
};

//----------------------------------------------------------------

template<class ValueType, typename Callback, class SourceObjectType>
inline AutoDeltaVariableObserver<ValueType, Callback, SourceObjectType>::AutoDeltaVariableObserver() :
AutoDeltaVariable<ValueType>(),
sourceObject(0)
{
}

//----------------------------------------------------------------

template<class ValueType, typename Callback, class SourceObjectType>
inline AutoDeltaVariableObserver<ValueType, Callback, SourceObjectType>::AutoDeltaVariableObserver(const AutoDeltaVariableObserver & source) :
AutoDeltaVariable<ValueType>(source),
sourceObject(0)
{
}

//----------------------------------------------------------------

template<class ValueType, typename Callback, class SourceObjectType>
inline AutoDeltaVariableObserver<ValueType, Callback, SourceObjectType>::AutoDeltaVariableObserver(const ValueType & source) : 
AutoDeltaVariable<ValueType>(source),
sourceObject(0)
{
}

//----------------------------------------------------------------

template<class ValueType, typename Callback, class SourceObjectType>
inline AutoDeltaVariableObserver<ValueType, Callback, SourceObjectType>::~AutoDeltaVariableObserver()
{
}

//----------------------------------------------------------------

template<class ValueType, typename Callback, class SourceObjectType>
inline AutoDeltaVariableObserver<ValueType, Callback, SourceObjectType> & AutoDeltaVariableObserver<ValueType, Callback, SourceObjectType>::operator =(const AutoDeltaVariableObserver<ValueType, Callback, SourceObjectType> & rhs) 
{
	if (&rhs != this)
		set(rhs.get());
	return *this;
}

//-----------------------------------------------------------------------

template<class ValueType, typename Callback, class SourceObjectType>
inline AutoDeltaVariableObserver<ValueType, Callback, SourceObjectType> & AutoDeltaVariableObserver<ValueType, Callback, SourceObjectType>::operator = (const ValueType & rhs)
{
	if (rhs != this->get())
		set(rhs);
	return *this;
}

//-----------------------------------------------------------------------

template<class ValueType, typename Callback, class SourceObjectType>
inline void AutoDeltaVariableObserver<ValueType, Callback, SourceObjectType>::setSourceObject(SourceObjectType * source)
{
	sourceObject = source;
}

//-----------------------------------------------------------------------

template<class ValueType, typename Callback, class SourceObjectType>
inline void AutoDeltaVariableObserver<ValueType, Callback, SourceObjectType>::set(const ValueType & source)
{
	Callback callback(sourceObject, ADOO_set);
	AutoDeltaVariable<ValueType>::set(source);
}

//-----------------------------------------------------------------------

template<class ValueType, typename Callback, class SourceObjectType>
inline void AutoDeltaVariableObserver<ValueType, Callback, SourceObjectType>::unpack(ReadIterator & source)
{
	Callback callback(sourceObject, ADOO_unpack);
	AutoDeltaVariable<ValueType>::unpack(source);
}

//-----------------------------------------------------------------------

template<class ValueType, typename Callback, class SourceObjectType>
inline void AutoDeltaVariableObserver<ValueType, Callback, SourceObjectType>::unpackDelta(ReadIterator & source)
{
	Callback callback(sourceObject, ADOO_unpackDelta);
	AutoDeltaVariable<ValueType>::unpackDelta(source);
}

//-----------------------------------------------------------------------

}

#endif

