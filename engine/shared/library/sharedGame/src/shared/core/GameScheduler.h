// ======================================================================
//
// GameScheduler.h
// Copyright 2002 Sony Online Entertainment Inc.
// All Rights Reserved.
// 
// ======================================================================

#ifndef INCLUDED_GameScheduler_H
#define INCLUDED_GameScheduler_H

// ======================================================================

class GameScheduler
{
public:

	typedef void (*Callback)(const void *context);

public:

	static void install();

	static void addCallback(Callback callback, const void *context, float elapsedTime);
	static void alter(float elapsedTime);

private:

	static void remove();

};

// ======================================================================

#endif

