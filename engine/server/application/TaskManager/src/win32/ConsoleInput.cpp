// ConsoleInput.cpp
// Copyright 2000-02, Sony Online Entertainment Inc., all rights reserved. 
// Author: Justin Randall

//-----------------------------------------------------------------------

#include "FirstTaskManager.h"
#include "Console.h"
#include <conio.h>

//-----------------------------------------------------------------------

const char Console::getNextChar()
{
	char result = 0;
	if(_kbhit())
		result = static_cast<char>(_getche());
	return result;
}

//-----------------------------------------------------------------------

