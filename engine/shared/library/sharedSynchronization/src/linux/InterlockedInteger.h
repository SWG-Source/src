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
	operator int() const { return value; }
private:
	InterlockedInteger(const InterlockedInteger &o);
	InterlockedInteger &operator =(const InterlockedInteger &o);
	pthread_mutex_t lock;
	volatile int value;
};

inline InterlockedInteger::InterlockedInteger(int i_value)
: value(i_value)
{
  pthread_mutex_init(&lock, 0);
}

inline int InterlockedInteger::operator =(int i_value)
{
	pthread_mutex_lock(&lock);
	int oldvalue = value;
	value = i_value;
	pthread_mutex_unlock(&lock);
	return oldvalue;
}

inline int InterlockedInteger::operator ++()
{
	pthread_mutex_lock(&lock);
	int newvalue = value+1;
	value = newvalue;
	pthread_mutex_unlock(&lock);
	return newvalue;
}

inline int InterlockedInteger::operator --()
{
	pthread_mutex_lock(&lock);
	int newvalue = value-1;
	value = newvalue;
	pthread_mutex_unlock(&lock);
	return newvalue;
}

#endif
