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

	typedef __int64 Type;

public:

	static void install();
	static void getTime(Type &time);
	static void getCalibratedTime(Type &time, Type &frequency);
	static void getFrequency(Type &frequency);
};

// ======================================================================

#endif
