// ======================================================================
//
// WriteOnce.h
// Acy Stapp
//
// Copyright 6/19/2001 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_WriteOnce_h
#define INCLUDED_WriteOnce_h

#include "sharedSynchronization/Gate.h"

template <class T>
class WriteOnce
{
public:
	WriteOnce(): gate(false) {}
	~WriteOnce() {}
	explicit WriteOnce(T &i_initial): gate(false), value(i_initial) {}
	T &operator =(const T &i_value) { value = i_value; gate.open(); return value; }
	operator T() { gate.wait(); return value; }
private:
	WriteOnce(const WriteOnce &o);
	WriteOnce &operator =(const WriteOnce &o);
	Gate gate;
	T value;
};

#endif