#ifndef	_INCLUDED_AutoDeltaVectorObserver_H
#define	_INCLUDED_AutoDeltaVectorObserver_H

#include "AutoDeltaVector.h"
#include "AutoDeltaObserverOps.h"

namespace Archive {

/**
	@brief an authoritative variable that provides notification both prior to and after change
*/

template<typename ValueType, typename Callback, class SourceObjectType, typename ObjectType=DefaultObjectType>
class AutoDeltaVectorObserver : public AutoDeltaVector<ValueType, ObjectType>
{
public:

	typedef std::vector<ValueType> VectorType;

	AutoDeltaVectorObserver();
	AutoDeltaVectorObserver(const AutoDeltaVectorObserver & source);
	explicit AutoDeltaVectorObserver(const ValueType & source);
	~AutoDeltaVectorObserver();

	AutoDeltaVectorObserver & operator = (const AutoDeltaVectorObserver & source);
	AutoDeltaVectorObserver & operator = (const ValueType & source);

	void                 setSourceObject(SourceObjectType * source);

	virtual void         clear         ();
	virtual void         erase         (const unsigned int element);
	virtual void         pop_back      ();
	virtual void         push_back     (const ValueType & t);
	virtual void         insert        (const unsigned int before, const ValueType &newValue);
	virtual void         set           (const unsigned int element, const ValueType &source);
	virtual void         set           (const VectorType & newValue);
	virtual void         unpack        (ReadIterator & source);
	virtual void         unpackDelta   (ReadIterator & source);

private:
	SourceObjectType*    sourceObject;
};

//----------------------------------------------------------------

template<typename ValueType, typename Callback, class SourceObjectType, typename ObjectType>
inline AutoDeltaVectorObserver<ValueType, Callback, SourceObjectType, ObjectType>::AutoDeltaVectorObserver() :
AutoDeltaVector<ValueType, ObjectType>(),
sourceObject(0)
{
}

//----------------------------------------------------------------

template<typename ValueType, typename Callback, class SourceObjectType, typename ObjectType>
inline AutoDeltaVectorObserver<ValueType, Callback, SourceObjectType, ObjectType>::AutoDeltaVectorObserver(const AutoDeltaVectorObserver & source) :
AutoDeltaVector<ValueType, ObjectType>(source),
sourceObject(0)
{
}

//----------------------------------------------------------------

template<typename ValueType, typename Callback, class SourceObjectType, typename ObjectType>
inline AutoDeltaVectorObserver<ValueType, Callback, SourceObjectType, ObjectType>::AutoDeltaVectorObserver(const ValueType & source) : 
AutoDeltaVector<ValueType, ObjectType>(source),
sourceObject(0)
{
}

//----------------------------------------------------------------

template<typename ValueType, typename Callback, class SourceObjectType, typename ObjectType>
inline AutoDeltaVectorObserver<ValueType, Callback, SourceObjectType, ObjectType>::~AutoDeltaVectorObserver()
{
}

//----------------------------------------------------------------

template<typename ValueType, typename Callback, class SourceObjectType, typename ObjectType>
inline AutoDeltaVectorObserver<ValueType, Callback, SourceObjectType, ObjectType> & AutoDeltaVectorObserver<ValueType, Callback, SourceObjectType, ObjectType>::operator =(const AutoDeltaVectorObserver<ValueType, Callback, SourceObjectType, ObjectType> & rhs) 
{
	if (&rhs != this)
		set(rhs.get());
	return *this;
}

//-----------------------------------------------------------------------

template<typename ValueType, typename Callback, class SourceObjectType, typename ObjectType>
inline AutoDeltaVectorObserver<ValueType, Callback, SourceObjectType, ObjectType> & AutoDeltaVectorObserver<ValueType, Callback, SourceObjectType, ObjectType>::operator = (const ValueType & rhs)
{
	if (rhs != this->get())
		set(rhs);
	return *this;
}

//-----------------------------------------------------------------------

template<typename ValueType, typename Callback, class SourceObjectType, typename ObjectType>
inline void AutoDeltaVectorObserver<ValueType, Callback, SourceObjectType, ObjectType>::setSourceObject(SourceObjectType * source)
{
	sourceObject = source;
}

//-----------------------------------------------------------------------

template<typename ValueType, typename Callback, class SourceObjectType, typename ObjectType>
inline void AutoDeltaVectorObserver<ValueType, Callback, SourceObjectType, ObjectType>::clear()
{
	Callback callback(sourceObject, ADOO_clear);
	AutoDeltaVector<ValueType, ObjectType>::clear();
}

//-----------------------------------------------------------------------

template<typename ValueType, typename Callback, class SourceObjectType, typename ObjectType>
inline void AutoDeltaVectorObserver<ValueType, Callback, SourceObjectType, ObjectType>::erase(const unsigned int element)
{
	Callback callback(sourceObject, ADOO_erase);
	AutoDeltaVector<ValueType, ObjectType>::erase(element);
}

//-----------------------------------------------------------------------

template<typename ValueType, typename Callback, class SourceObjectType, typename ObjectType>
inline void AutoDeltaVectorObserver<ValueType, Callback, SourceObjectType, ObjectType>::pop_back()
{
	Callback callback(sourceObject, ADOO_pop_back);
	AutoDeltaVector<ValueType, ObjectType>::pop_back();
}

//-----------------------------------------------------------------------

template<typename ValueType, typename Callback, class SourceObjectType, typename ObjectType>
inline void AutoDeltaVectorObserver<ValueType, Callback, SourceObjectType, ObjectType>::push_back(const ValueType & t)
{
	Callback callback(sourceObject, ADOO_push_back);
	AutoDeltaVector<ValueType, ObjectType>::push_back(t);
}

//-----------------------------------------------------------------------

template<typename ValueType, typename Callback, class SourceObjectType, typename ObjectType>
inline void AutoDeltaVectorObserver<ValueType, Callback, SourceObjectType, ObjectType>::insert(const unsigned int before, const ValueType &newValue)
{
	Callback callback(sourceObject, ADOO_insert);
	AutoDeltaVector<ValueType, ObjectType>::insert(before, newValue);
}

//-----------------------------------------------------------------------

template<typename ValueType, typename Callback, class SourceObjectType, typename ObjectType>
inline void AutoDeltaVectorObserver<ValueType, Callback, SourceObjectType, ObjectType>::set(const unsigned int element, const ValueType & source)
{
	Callback callback(sourceObject, ADOO_set);
	AutoDeltaVector<ValueType, ObjectType>::set(element, source);
}

//-----------------------------------------------------------------------

template<typename ValueType, typename Callback, class SourceObjectType, typename ObjectType>
inline void AutoDeltaVectorObserver<ValueType, Callback, SourceObjectType, ObjectType>::set(const VectorType &newValue)
{
	Callback callback(sourceObject, ADOO_setAll);
	AutoDeltaVector<ValueType, ObjectType>::set(newValue);
}

//-----------------------------------------------------------------------

template<typename ValueType, typename Callback, class SourceObjectType, typename ObjectType>
inline void AutoDeltaVectorObserver<ValueType, Callback, SourceObjectType, ObjectType>::unpack(ReadIterator & source)
{
	Callback callback(sourceObject, ADOO_unpack);
	AutoDeltaVector<ValueType, ObjectType>::unpack(source);
}

//-----------------------------------------------------------------------

template<typename ValueType, typename Callback, class SourceObjectType, typename ObjectType>
inline void AutoDeltaVectorObserver<ValueType, Callback, SourceObjectType, ObjectType>::unpackDelta(ReadIterator & source)
{
	Callback callback(sourceObject, ADOO_unpackDelta);
	AutoDeltaVector<ValueType, ObjectType>::unpackDelta(source);
}

//-----------------------------------------------------------------------

}

#endif


