//=================================================================== //
//
// PerformanceTimer.h
// copyright 2000-2004 Sony Online Entertainment
// All Rights Reserved
//
//===================================================================

#ifndef INCLUDED_PerformaceTimer_H
#define INCLUDED_PerformaceTimer_H

//===================================================================

#include <unistd.h>
#include <sys/time.h>

//===================================================================

class PerformanceTimer
{
public:

	static void install();

public:

	PerformanceTimer();
	~PerformanceTimer();

	void start();
	void resume();
	void stop();

	float getElapsedTime() const;
	float getSplitTime() const;
	void logElapsedTime(const char* string) const;

private:

	timeval startTime;
	timeval stopTime;

private:

	PerformanceTimer(PerformanceTimer const &);
	PerformanceTimer & operator=(PerformanceTimer const &);
};

//===================================================================

#endif
