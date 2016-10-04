// ======================================================================
//
// DebugMonitor.cpp
// Copyright 2002, Sony Online Entertainment.h
// All rights reserved.
//
// ======================================================================

#include "sharedDebug/FirstSharedDebug.h"
#include "sharedDebug/DebugMonitor.h"

#ifdef _DEBUG

#include "sharedDebug/ConfigSharedDebugLinux.h"
#include "sharedDebug/Profiler.h"
#include "sharedFoundation/ConfigSharedFoundation.h"

#include <curses.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <termios.h>

// ======================================================================

namespace DebugMonitorNamespace
{
	bool    s_installed;
	SCREEN *s_initialScreen;
	WINDOW *s_initialWindow;

	FILE   *s_ttyOutputFile;
	FILE   *s_ttyInputFile;
	SCREEN *s_outputScreen;
	WINDOW *s_outputWindow;
}

using namespace DebugMonitorNamespace;

// ======================================================================
// Install the debug monitor subsystem
//
// Remarks:
//
//   This routine will first attempt to install the selected debug monitor.

void DebugMonitor::install(void)
{
	DEBUG_FATAL(s_installed, ("DebugMonitor already installed."));

	//-- Check if user wants to use a TTY for the DebugMonitor.
	if (!ConfigSharedDebugLinux::getUseTty())
	{
		// Do not use the DebugMonitor.
		DEBUG_REPORT_LOG(ConfigSharedDebugLinux::getLogTtySetup(), ("DebugMonitor: skipping install, ConfigSharedDebugLinux/useTty is false.\n"));
		return;
	}

	//-- Open and retrieve the name of the TTY file for output from the TTY Specification file.
	char const *const configFilename = ConfigSharedDebugLinux::getDebugMonitorOutputFilename();

	if (!configFilename || (*configFilename == '\0'))
	{
		DEBUG_WARNING(true, ("DebugMonitor(): skipping installation, no debugMonitorOutputFilename specified."));
		return;
	}

	char ttyFilename[1024];

	if (configFilename[0] != '@')
	{
		// filename is the real output filename (usually a device) for DebugMonitor output.
		strncpy(ttyFilename, configFilename, sizeof(ttyFilename) - 1);
		ttyFilename[sizeof(ttyFilename)-1] = 0;
	}
	else
	{
		// filename is actually an indirection filename, the contents of this file contains the real filename.
		FILE *ttySpecifierFile = fopen(configFilename + 1, "r");
		if (!ttySpecifierFile)
		{
			DEBUG_WARNING(true, ("DebugMonitor: failed to open tty specifier file [%s]: [%s].", configFilename + 1, strerror(errno)));
			return;
		}

		// Retrieve the TTY file name.
		fgets(ttyFilename, sizeof(ttyFilename), ttySpecifierFile);
		fclose(ttySpecifierFile);
	}
		
	// Strip trailing newline from TTY filename.
	const int filenameLength = strlen(ttyFilename);
	if (filenameLength > 0)
	{
		// chop off trailing newline if present.
		if (ttyFilename[filenameLength - 1] == '\n')
			ttyFilename[filenameLength - 1] = '\0';
	}

	//-- Open the TTY file for output.
	DEBUG_REPORT_LOG(ConfigSharedDebugLinux::getLogTtySetup(), ("DebugMonitor: opening TTY file [%s] for reading and writing.\n", ttyFilename));
	FILE *s_ttyOutputFile = fopen(ttyFilename, "w");
	if (!s_ttyOutputFile)
	{
		DEBUG_WARNING(true, ("DebugMonitor: failed to open tty file for output [%s]: [%s].", ttyFilename, strerror(errno)));
		return;
	}

	//-- Open the TTY file for input.
	FILE *s_ttyInputFile = fopen(ttyFilename, "r");
	if (!s_ttyInputFile)
	{
		DEBUG_WARNING(true, ("DebugMonitor: failed to open tty file for input [%s]: [%s].", ttyFilename, strerror(errno)));
		fclose(s_ttyOutputFile);
		return;
	}

	// NOTE: -TRF- I would not expect this chunk of termios setup
	//       code to be necessary.  My expectation is that it should
	//       be handled by the curs_inopts (see man page) options I have
	//       selected below.  Alas, these are needed.
	
	//-- Setup profiler window input mode.
	const int inputFd = STDIN_FILENO;
	
	// Get terminal attributes.
	termios  terminalAttributes;
	if (tcgetattr(inputFd, &terminalAttributes) != 0)
	{
		DEBUG_WARNING(true, ("DebugMonitor: tcgetattr failed [%s].", strerror(errno)));
		fclose(s_ttyOutputFile);
		fclose(s_ttyInputFile);
		return;
	}
	
	// Turn off line-processing mode.  We want a character at a time.
	terminalAttributes.c_lflag &= ~ICANON;

	// Turn off echo.
	terminalAttributes.c_lflag &= ~ECHO;

	// Specify that read should return immediately (non-blocking).
	terminalAttributes.c_cc[VMIN]  = 0;
	terminalAttributes.c_cc[VTIME] = 0;
	
	// Set terminal attributes.
	if (tcsetattr(inputFd, TCSAFLUSH, &terminalAttributes) != 0)
	{
		DEBUG_WARNING(true, ("DebugMonitor: tcsetattr failed [%s].", strerror(errno)));
		fclose(s_ttyOutputFile);
		fclose(s_ttyInputFile);
		return;
	}
	
	//-- Create a curses screen to represent the DebugMonitor output tty.
	//   NOTE: for now the curses input is hooked up to the application's
	//         standard input.  Later we may want to change that to
	//         handle input from the output terminal, particularly to
	//         handle profiler modifications when the output window
	//         is active.
	s_outputScreen = newterm(nullptr, s_ttyOutputFile, stdin);
	if (!s_outputScreen)
	{
		DEBUG_WARNING(true, ("DebugMonitor: newterm() failed [%s].", strerror(errno)));
		fclose(s_ttyOutputFile);
		fclose(s_ttyInputFile);
		return;
	}

	//-- Initialize the curses library.
	s_initialWindow = initscr();
	if (!s_initialWindow)
	{
		DEBUG_WARNING(true, ("DebugMonitor: initscr(): failed [%s].", strerror(errno)));
		fclose(s_ttyOutputFile);
		fclose(s_ttyInputFile);
		return;
	}

	//-- Restore the stdout window to its previous state.  We won't use stdout.
	endwin();

	//-- Set the default terminal to be the output TTY's terminal screen.
	s_initialScreen = set_term(s_outputScreen);
	if (!s_initialScreen)
	{
		DEBUG_WARNING(true, ("DebugMonitor: set_term(): failed [%s].", strerror(errno)));
		delscreen(s_outputScreen);
		fclose(s_ttyOutputFile);
		fclose(s_ttyInputFile);
		return;
	}

	//-- Create the curses WINDOW at the full size of the TTY.
	s_outputWindow = newwin(0, 0, 0, 0);
	if (!s_outputWindow)
	{
		DEBUG_WARNING(true, ("DebugMonitor: newwin(): failed [%s].", strerror(errno)));
		delscreen(s_outputScreen);
		fclose(s_ttyOutputFile);
		fclose(s_ttyInputFile);
		return;
	}

	//-- Destroy curses data structures associated with the stdout TTY
	delwin(s_initialWindow);
	delscreen(s_initialScreen);

	//-- Additional profiler input setup.
	// Allow translation of arrow keys.
	if (keypad(s_outputWindow, true) == ERR)
	{
		fclose(s_ttyOutputFile);
		fclose(s_ttyInputFile);
		DEBUG_WARNING(true, ("DebugMonitor: keypad() failed [%s].", strerror(errno)));
		return;
	}
	
	//-- Add the removal routine to the ExitChain.
	s_installed = true;
	
	// NOTE: this can't go on the exit chain because the MemoryManager
	//       will write to it way late in processing.
	//ExitChain::add(remove, "DebugMonitor");

	fclose(s_ttyOutputFile);
	fclose(s_ttyInputFile);
}

// ----------------------------------------------------------------------

void DebugMonitor::remove(void)
{
	if (!s_installed)
	{
		// Exit silently rather than FATAL.  There's any number of reasons why
		// we might not be installed, and the ExitChain does not call remove,
		// so this should be valid.
		return;
	}
	
	s_installed = false;

	//-- Restore the TTY's state. 	
	endwin();

	//-- Delete curses data structures associated with the output terminal.
	delwin(s_outputWindow);
	s_outputWindow = 0;

	delscreen(s_outputScreen);
	s_outputScreen = 0;

	//-- Close file handle to output terminal.
	fclose(s_ttyOutputFile);
	s_ttyOutputFile = 0;

	fclose(s_ttyInputFile);
	s_ttyInputFile = 0;
}

// ----------------------------------------------------------------------
/**
 * Set the debug window's z-order.
 */

void DebugMonitor::setBehindWindow(HWND window)
{
	UNREF(window);
}

// ----------------------------------------------------------------------
/**
 * Clear the debug monitor and home the cursor.
 * 
 * If the mono monitor is not installed, this routine does nothing.
 * 
 * This routine will clear the contents of the debug monitor, reset the screen
 * offset to 0, and move the cursor to the upper left corner of the screen.
 * 
 * @see DebugMonitor::home(), DebugMonitor::clearToCursor()
 */

void DebugMonitor::clearScreen(void)
{
	//-- Ignore request if DebugMonitor isn't installed.
	if (!s_installed)
		return;

	//-- Tell curses to clear the virtual screen.  The actual
	//   clear will not take effect until the next showOutput() call
	//   is made.
	IGNORE_RETURN(wclear(s_outputWindow));

	//-- Goto upper left corner.
	gotoXY(0, 0);
}

// ----------------------------------------------------------------------
/**
 * Clear the debug monitor to the current cursor position and home the cursor.
 * 
 * If the debug monitor is not installed, this routine does nothing.
 * 
 * This routine will clear the contents of the debug monitor only up to the
 * cursor position.  If the cursor is not very far down on the screen,
 * this routine may be significantly more efficient clearing the screen.
 * 
 * It will also move the cursor to the upper left corner of the screen.
 * 
 * @see DebugMonitor::clearScreen(), DebugMonitor::home()
 */

void DebugMonitor::clearToCursor(void)
{
	//-- Ignore request if DebugMonitor isn't installed.
	if (!s_installed)
		return;
	
	//-- I don't see this functionality in curses.  Just clear
	//   the entire screen.
	clearScreen();
}

// ----------------------------------------------------------------------
/**
 * Position the cursor on the debug monitor screen.
 * 
 * If the debug monitor is not installed, this routine does nothing.
 * 
 * All printing happens at the cursor position.
 * 
 * @param x  New X position for the cursor
 * @param y  New Y position for the cursor
 */

void DebugMonitor::gotoXY(int x, int y)
{
	//-- Ignore request if DebugMonitor isn't installed.
	if (!s_installed)
		return;

	//-- Move the cursor position to the specified location.
	const int result = wmove(s_outputWindow, y, x);
	DEBUG_REPORT_LOG(result == ERR, ("DebugMonitor: wmove(%d, %d) failed [%s].\n", y, x, strerror(errno)));
	UNREF(result);
}

// ----------------------------------------------------------------------
/**
 * Display a string on the debug monitor.
 * * If the debug monitor is not installed, this routine does nothing.
 * 
 * Printing occurs from the cursor position.
 * 
 * Newline characters '\n' will cause the cursor position to advance to the
 * beginning of the next line.  If the cursor is already on the last line of
 * the screen, the screen will scroll up one line and the cursor will move to
 * the beginning of the last line.
 * 
 * The backspace character '\b' will cause the cursor to move one character
 * backwards.  If at the beginning of the line, the cursor will move to the
 * end of the previous line.  If already on the first line of the screen, the
 * cursor position and screen contents will be unchanged.
 * 
 * All other characters are placed directly into the text frame buffer.
 * After each character, the cursor will be logically advanced one
 * character forward.  If the cursor was on the last column, it will advance
 * to the next line.  If the cursor was already on the last line, the screen
 * will be scrolled up one line and the cursor will move to the beginning of
 * the last line.
 * 
 * @param string  String to display on the debug monitor
 */

void DebugMonitor::print(const char *string)
{
	//-- Ignore request if DebugMonitor isn't installed.
	if (!s_installed)
		return;

	const int result = wprintw(s_outputWindow, const_cast<char*>(string));
	DEBUG_REPORT_LOG(result == ERR, ("WARNING: DebugMonitor: wprintw failed [%s].\n", strerror(errno)));
	UNREF(result);
}

// ----------------------------------------------------------------------
/**
 * Ensure all changes to the DebugMonitor have taken effect by the time
 * this function returns.
 *
 * Note: some platforms may do nothing here.  The Win32 platform does not
 *       require flushing.  The Linux platform does.  Call it assuming
 *       that it is needed.  It will be a no-op when not required.
 */

void DebugMonitor::flushOutput()
{
	//-- Ignore request if DebugMonitor isn't installed.
	if (!s_installed)
		return;

	//-- Tell curses to refresh the output window so that
	//   the virtual curses display is rendered to the real
	//   terminal display.
	const int result = wrefresh(s_outputWindow);
	DEBUG_REPORT_LOG(result == ERR, ("WARNING: DebugMonitor: wrefresh failed [%s].\n", strerror(errno)));
	UNREF(result);

	//-- Handle tty input here.  This is somewhat hacky.  Unix tty input
	//   and output currently are intimately tied to the profiler.  Later
	//   this can be separated out so we can handle multiple TTYs for
	//   multiple purposes, such as for handling a TTY-based popup debug
	//   menu for on-the-fly options changing.
	const int input = wgetch(s_outputWindow);
	if (input != ERR)
	{
		//-- Handle input.
		switch (input)
		{
			case '8':
			case KEY_UP:
				Profiler::selectionMoveUp();
				break;

			case '9':
			case KEY_DOWN:
				Profiler::selectionMoveDown();
				break;
				
			case '0':
			case 10:
			case KEY_LEFT:
			case KEY_RIGHT:
				Profiler::selectionToggleExpanded();
				break;

			default:
				// Nothing to do.
				break;
		}
	}
}

// ======================================================================

#endif

