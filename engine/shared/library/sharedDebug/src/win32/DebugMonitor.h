// ======================================================================
//
// DebugMonitor.h
//
// Portions copyright 1998 Bootprint Entertainment
// Portions copyright 2002-2004 Sony Online Entertainment
// All Rights Reserved.
//
// ======================================================================

#ifndef INCLUDED_DebugMonitor_H
#define INCLUDED_DebugMonitor_H

// ======================================================================

#include "sharedFoundation/Production.h"

// ======================================================================

#if PRODUCTION == 0

class DebugMonitor
{
public:

	static void install();
	static void remove();

	static void setBehindWindow(HWND window);

	static void show();
	static void hide();

	static void clearScreen();
	static void clearToCursor();

	static void home();
	static void gotoXY(int x, int y);
	static void print(const char *string);

	static void flushOutput();
};

#endif

// ======================================================================

#endif
