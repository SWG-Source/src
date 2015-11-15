// ======================================================================
//
// ProfilerTimer.h
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_ProfilerTimer_H
#define INCLUDED_ProfilerTimer_H

// ======================================================================

class ProfilerTimer
{
public:

	typedef uint64 Type;

public:

	static void install();
	static void getTime(Type &type);
	static void getCalibratedTime(Type &time, Type &frequency);
	static void getFrequency(Type &frequency);
};

// ======================================================================

#endif
