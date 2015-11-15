// ======================================================================
//
// BlockingPointer.h
// Acy Stapp
//
// Copyright 6/19/2001 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_BlockingPointer_h
#define INCLUDED_BlockingPointer_h

#include "sharedSynchronization/Mutex.h"
#include "sharedSynchronization/Semaphore.h"

template<class T>
class BlockingPointer
{
public:
	BlockingPointer(Mutex &m, T * begin, T * end);
	T * operator ++(int);
	T * operator --();
private:
	BlockingPointer(const BlockingPointer &o);
	BlockingPointer &operator =(const BlockingPointer &o);
	Mutex &_mutex;
	T * volatile value;
	T * begin, * end;
	Semaphore high, low;
};

template <class T>
BlockingPointer<T>::BlockingPointer<T>(Mutex &m, T * i_begin, T * i_end)
: _mutex(m), value(i_begin), begin(i_begin), end(i_end)
{
}

template <class T>
T * BlockingPointer<T>::operator ++(int)
{
	++value;
	if (value == begin)
	{
		low.release();
	}
	else
	{
		if (value > end)
		{
			_mutex.leave();
			high.wait();
			_mutex.enter();
		}
	}
	return value-1;
}

template <class T>
T * BlockingPointer<T>::operator --()
{
	--value;
	if (value == end - 1)
	{
		high.release();
	}
	else
	{
		if (value < begin)
		{
			_mutex.leave();
			low.wait();
			_mutex.enter();
		}
	}
	return value;
}

#endif