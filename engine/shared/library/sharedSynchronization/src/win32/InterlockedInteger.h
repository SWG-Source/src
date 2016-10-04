// ======================================================================
//
// InterlockedInteger.h
// Acy Stapp
//
// Copyright 6/19/2001 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_InterlockedInteger_h
#define INCLUDED_InterlockedInteger_h

// %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

class InterlockedInteger
{
public:
	explicit InterlockedInteger(int initialValue=0);
	int operator =(int); // returns prior value (exchange)
	int operator ++(); // returns new value
	int operator --(); // returns new value
	operator int() const { return static_cast<int>(value); }
private:
	InterlockedInteger(const InterlockedInteger &o);
	InterlockedInteger &operator =(const InterlockedInteger &o);
	volatile int value;
};

inline InterlockedInteger::InterlockedInteger(int i_value)
: value(i_value)
{
}

inline int InterlockedInteger::operator =(int i_value)
{
	long * ptr = (long *)&value;
	return static_cast<int>(InterlockedExchange(ptr, static_cast<long>(i_value)));
}

inline int InterlockedInteger::operator ++()
{
	long * ptr = (long *)&value;
	return static_cast<int>(InterlockedIncrement(ptr));
}

inline int InterlockedInteger::operator --()
{
	long * ptr = (long *)&value;
	return static_cast<int>(InterlockedDecrement(ptr));
}

#endif