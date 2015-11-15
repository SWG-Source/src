// ======================================================================
//
// AutoDeltaSetObserver.h
//
// Copyright 2003 Sony Online Entertainment
//
// ======================================================================

#ifndef	_INCLUDED_AutoDeltaSetObserver_H
#define	_INCLUDED_AutoDeltaSetObserver_H

// ======================================================================

#include "AutoDeltaSet.h"
#include "AutoDeltaObserverOps.h"

// ----------------------------------------------------------------------

namespace Archive {

// ----------------------------------------------------------------------

template<typename ValueType, typename Callback, class SourceObjectType, typename ObjectType = DefaultObjectType>
class AutoDeltaSetObserver: public AutoDeltaSet<ValueType, ObjectType>
{
public:
	typedef std::set<ValueType> SetType;
	typedef typename SetType::const_iterator const_iterator;

	AutoDeltaSetObserver();
	~AutoDeltaSetObserver();

	void                 setSourceObject(SourceObjectType *source);

	virtual void         clear         ();
	virtual typename SetType::const_iterator erase(ValueType const &value);
	virtual typename SetType::const_iterator erase(const_iterator & i);
	virtual void         insert        (ValueType const &value);
	virtual void         unpack        (ReadIterator &source);
	virtual void         unpackDelta   (ReadIterator &source);

private:
	AutoDeltaSetObserver(AutoDeltaSetObserver const &);
	AutoDeltaSetObserver &operator=(AutoDeltaSetObserver const &);

private:
	SourceObjectType *sourceObject;
};

//----------------------------------------------------------------

template<typename ValueType, typename Callback, class SourceObjectType, typename ObjectType>
inline AutoDeltaSetObserver<ValueType, Callback, SourceObjectType, ObjectType>::AutoDeltaSetObserver() :
	AutoDeltaSet<ValueType, ObjectType>(),
	sourceObject(0)
{
}

//----------------------------------------------------------------

template<typename ValueType, typename Callback, class SourceObjectType, typename ObjectType>
inline AutoDeltaSetObserver<ValueType, Callback, SourceObjectType, ObjectType>::~AutoDeltaSetObserver()
{
}

//-----------------------------------------------------------------------

template<typename ValueType, typename Callback, class SourceObjectType, typename ObjectType>
inline void AutoDeltaSetObserver<ValueType, Callback, SourceObjectType, ObjectType>::setSourceObject(SourceObjectType *source)
{
	sourceObject = source;
}

//-----------------------------------------------------------------------

template<typename ValueType, typename Callback, class SourceObjectType, typename ObjectType>
inline void AutoDeltaSetObserver<ValueType, Callback, SourceObjectType, ObjectType>::clear()
{
	Callback callback(sourceObject, ADOO_clear);
	AutoDeltaSet<ValueType, ObjectType>::clear();
}

//-----------------------------------------------------------------------

template<typename ValueType, typename Callback, class SourceObjectType, typename ObjectType>
inline typename AutoDeltaSetObserver<ValueType, Callback, SourceObjectType, ObjectType>::const_iterator AutoDeltaSetObserver<ValueType, Callback, SourceObjectType, ObjectType>::erase(ValueType const &value)
{
	Callback callback(sourceObject, ADOO_erase);
	return AutoDeltaSet<ValueType, ObjectType>::erase(value);
}

//-----------------------------------------------------------------------

template<typename ValueType, typename Callback, class SourceObjectType, typename ObjectType>
inline typename AutoDeltaSetObserver<ValueType, Callback, SourceObjectType, ObjectType>::const_iterator AutoDeltaSetObserver<ValueType, Callback, SourceObjectType, ObjectType>::erase(const_iterator & i)
{
	Callback callback(sourceObject, ADOO_erase);
	return AutoDeltaSet<ValueType, ObjectType>::erase(i);
}

//-----------------------------------------------------------------------

template<typename ValueType, typename Callback, class SourceObjectType, typename ObjectType>
inline void AutoDeltaSetObserver<ValueType, Callback, SourceObjectType, ObjectType>::insert(ValueType const &value)
{
	Callback callback(sourceObject, ADOO_insert);
	AutoDeltaSet<ValueType, ObjectType>::insert(value);
}

//-----------------------------------------------------------------------

template<typename ValueType, typename Callback, class SourceObjectType, typename ObjectType>
inline void AutoDeltaSetObserver<ValueType, Callback, SourceObjectType, ObjectType>::unpack(ReadIterator &source)
{
	Callback callback(sourceObject, ADOO_unpack);
	AutoDeltaSet<ValueType, ObjectType>::unpack(source);
}

//-----------------------------------------------------------------------

template<typename ValueType, typename Callback, class SourceObjectType, typename ObjectType>
inline void AutoDeltaSetObserver<ValueType, Callback, SourceObjectType, ObjectType>::unpackDelta(ReadIterator &source)
{
	Callback callback(sourceObject, ADOO_unpackDelta);
	AutoDeltaSet<ValueType, ObjectType>::unpackDelta(source);
}

//-----------------------------------------------------------------------

}

// ======================================================================

#endif

