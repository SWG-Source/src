// ======================================================================
//
// Mutex.h
// Acy Stapp
//
// Copyright 6/19/2001 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_Mutex_h
#define INCLUDED_Mutex_h

class Mutex
{
public:
	Mutex();
	~Mutex();

	void enter();
	void leave();

	pthread_mutex_t &getInternalMutex() { return mutex; }
private:
	Mutex(const Mutex &o);
	Mutex &operator =(const Mutex &o);

	pthread_mutex_t mutex;
};


#endif
