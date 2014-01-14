// ======================================================================
//
// BlockingQueue.h
// Acy Stapp
//
// Copyright 6/19/2001 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_BlockingQueue_h
#define INCLUDED_BlockingQueue_h

template<class T>
class BlockingQueue
{
public:
	BlockingQueue(Mutex &m, T * begin, T * end);
	T * operator ++(int);
	T * operator --();
private:
	BlockingQueue(const BlockingQueue &o);
	BlockingQueue &operator =(const BlockingQueue &o);
	Mutex &_mutex;
	T * volatile first, * volatile last;
	T * begin, * end;
	int size, maxsize;
	Semaphore notFull, notEmpty;
};

template <class T>
BlockingQueue<T>::BlockingQueue<T>(Mutex &m, T * i_begin, T * i_end)
: _mutex(m), first(i_begin), last(i_begin), begin(i_begin), end(i_end), size(0), maxsize(i_end - i_begin)
{
}

template <class T>
T * BlockingQueue<T>::operator ++(int)
{
	++size;
	if (size > maxsize)
	{
		_mutex.leave();
		notFull.wait();
		_mutex.enter();
	}

	T * oldlast = last;
	++last;
	if (last == end)
	{
		last = begin;
	}

	if (size == 1)
	{
		notEmpty.signal();
	}

	return oldlast;
}

template <class T>
T * BlockingQueue<T>::operator --()
{
	--size;
	if (size == 0)
	{
		_mutex.leave();
		notEmpty.wait();
		_mutex.enter();
	}

	++first;
	if (first == end)
	{
		first = begin;
	}

	if (size == maxsize - 1)
	{
		notFull.signal();
	}

	return first;
}

#endif