// ======================================================================
//
// DebugMonitor.h
// Portions Copyright 1998, Bootprint Entertainment, Inc.
// Portions Copyright 2002, Sony Online Entertainment, Inc.
//
// ======================================================================

#ifndef INCLUDED_DebugMonitor_H
#define INCLUDED_DebugMonitor_H

// ======================================================================

#ifdef _DEBUG

class DebugMonitor
{
private:

public:

	static void install(void);
	static void remove(void);

	typedef void* HWND;

	static void setBehindWindow(HWND window);

	static void clearScreen(void);
	static void clearToCursor(void);

	static void home(void);
	static void gotoXY(int x, int y);
	static void print(const char *string);

	static void flushOutput();
};

#endif

// ======================================================================
// Move the cursor to the upper left hand corner of the mono monitor screen
//
// Remarks:
//
//   If the debug monitor is not installed, this routine does nothing.
//
//   All printing happens at the cursor position.
//
//   This routine is identical to calling gotoXY(0,0);
//
// See Also:
//
//   DebugMonitor::gotoXY()

#ifdef _DEBUG

inline void DebugMonitor::home(void)
{
	gotoXY(0,0);
}

#endif

// ======================================================================

#endif
