// ======================================================================
//
// Gate.h
//
// Copyright 2001-2002 Sony Online Entertainment
// All Rights Reseved.
//
// ======================================================================

#ifndef INCLUDED_Gate_h
#define INCLUDED_Gate_h

// ======================================================================

class Gate
{
public:

	Gate(bool initiallyOpen);
	~Gate();

	void wait();

	void close();
	void open();

private:

	Gate(const Gate &o);
	Gate &operator =(const Gate &o);

private:

	HANDLE handle;
};

// ======================================================================

#endif