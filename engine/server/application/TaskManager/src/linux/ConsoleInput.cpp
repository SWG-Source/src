// TaskManager.cpp
// Copyright 2000-01, Sony Online Entertainment Inc., all rights reserved. 
// Author: Justin Randall

//-----------------------------------------------------------------------

#include "FirstTaskManager.h"
#include "Console.h"
#include <stdio.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <curses.h>

//-----------------------------------------------------------------------

struct SetBufferMode
{
	SetBufferMode();
	~SetBufferMode() {};
};

SetBufferMode::SetBufferMode()
{
	setvbuf(stdin, nullptr, _IONBF, 0);
}

//-----------------------------------------------------------------------

const char Console::getNextChar()
{
	static SetBufferMode setBufferMode;
	char result = 0;
	fd_set rfds;
	struct timeval tv;

	FD_ZERO(&rfds);
	FD_SET(STDIN_FILENO, &rfds);

	tv.tv_sec = 0;
	tv.tv_usec = 0;

	int ready = select(1, &rfds, 0, 0, &tv);
	if(ready)
	{
		result = static_cast<char>(getchar());
		if(result < 1)
			result = 0;
	}
	/*
	if(_kbhit())
		result = static_cast<char>(_getche());
	*/
	return result;
}

//-----------------------------------------------------------------------

