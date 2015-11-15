//
// PerformanceTimer.h
// Copyright 2000-2004 Sony Online Entertainment
//

//-------------------------------------------------------------------

#ifndef INCLUDED_PerformanceTimer_H
#define INCLUDED_PerformanceTimer_H

//-------------------------------------------------------------------

class PerformanceTimer
{
public:

	static void install();

public:

	DLLEXPORT PerformanceTimer();
	DLLEXPORT ~PerformanceTimer();

	void DLLEXPORT start();
	void DLLEXPORT resume();
	void DLLEXPORT stop();

	float DLLEXPORT getElapsedTime() const;
	float getSplitTime() const;  // Get the time since the timer was started without stopping the timer.
	void logElapsedTime(const char* string) const;

private:

	PerformanceTimer(PerformanceTimer const &);
	PerformanceTimer & operator=(PerformanceTimer const &);

private:

	static __int64 ms_frequency;

private:

	__int64 m_startTime;
	__int64 m_stopTime;
};

//-------------------------------------------------------------------

#endif