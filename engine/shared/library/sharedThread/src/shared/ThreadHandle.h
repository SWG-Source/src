// ======================================================================
//
// ThreadHandle.h
// Acy Stapp
//
// Copyright 6/19/2001 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_ThreadHandle_h
#define INCLUDED_ThreadHandle_h

// ======================================================================

#include "sharedThread/Thread.h"

// ======================================================================

template <class T>
class TypedThreadHandle
{
public:
	typedef TypedThreadHandle<Thread> ThreadHandle;

	TypedThreadHandle();
	~TypedThreadHandle();
	explicit TypedThreadHandle(T * i);
	TypedThreadHandle(const TypedThreadHandle<T> &o);

	template <class O>
	TypedThreadHandle<T> &operator =(const TypedThreadHandle<O> &o)
	{
		O * oimpl = o.getImpl();
		if (impl != oimpl)
		{
			if (oimpl)
				oimpl->ref();
			if (impl)
				impl->deref();
			impl = oimpl;
		}
		return *this;
	}

	operator bool() const { return !(impl == 0); } //lint !e1930 conversion operator found
	bool operator !() const { return impl == 0; }
	void zero();
	void waitZero(); // wait then zero

	Thread * operator ->() { return impl; }
	const Thread * operator ->() const { return impl; }

	static ThreadHandle getCurrentThread();
	static ThreadHandle getMainThread();
	// TODO AJS - this is not needed once we get proper template friend declarations - friend template<class O> class TypedThreadHandle<O>;
	T * getImpl() const { return const_cast<T *>(impl); }
protected:
	T * impl;
}; 

typedef TypedThreadHandle<Thread> ThreadHandle;

template <class T>
TypedThreadHandle<T>::TypedThreadHandle()
: impl(0)
{
}

template <class T>
TypedThreadHandle<T>::TypedThreadHandle(T * i)
: impl(i)
{
	if (impl)
	{
		impl->ref();
		impl->start();
	}
}

template <class T>
TypedThreadHandle<T>::TypedThreadHandle(const TypedThreadHandle &o)
: impl(o.impl)
{
	if (impl)
		impl->ref();
}

template <class T>
TypedThreadHandle<T>::~TypedThreadHandle()
{
	if (impl)
		impl->deref();
	impl = 0;
}

template <class T>
TypedThreadHandle<Thread> TypedThreadHandle<T>::getCurrentThread()
{
	return TypedThreadHandle<Thread>(Thread::getCurrentThread());
}

template <class T>
TypedThreadHandle<Thread> TypedThreadHandle<T>::getMainThread()
{
	return TypedThreadHandle<Thread>(Thread::getMainThread());
}

template <class T>
void TypedThreadHandle<T>::waitZero()
{
	if (impl)
	{
		impl->wait();
		impl->deref();
	}
	impl = 0;
}

template <class T>
void TypedThreadHandle<T>::zero()
{
	if (impl)
		impl->deref();
	impl = 0;
}

#endif
