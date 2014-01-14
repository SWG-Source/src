// ======================================================================
//
// StderrLogger.cpp
//
// Copyright 2003 Sony Online Entertainment
//
// ======================================================================

#include "sharedLog/FirstSharedLog.h"
#include "sharedLog/StderrLogger.h"
#include "sharedLog/Log.h"
#include <string>
#include <fcntl.h>

// ======================================================================

namespace StderrLoggerNamespace
{
	bool s_installed;
	int s_oldStderr;
	int s_stderrPipe[2];
}
using namespace StderrLoggerNamespace;

// ======================================================================

void StderrLogger::install()
{
	DEBUG_FATAL(s_installed, ("StderrLogger already installed"));
	s_installed = true;
	s_oldStderr = dup(2);
	pipe(s_stderrPipe);
	dup2(s_stderrPipe[1], 2);
	fcntl(s_stderrPipe[0], F_SETFL, O_NONBLOCK);
}

// ----------------------------------------------------------------------

void StderrLogger::remove()
{
	DEBUG_FATAL(!s_installed, ("StderrLogger not installed"));
	dup2(s_oldStderr, 2);
	close(s_stderrPipe[0]);
	close(s_stderrPipe[1]);
	s_installed = false;
}

// ----------------------------------------------------------------------

void StderrLogger::update()
{
	if (!s_installed)
		return;

	char buf[8192];
	int nRead = read(s_stderrPipe[0], buf, sizeof(buf)-1);
	if (nRead > 0)
	{
		int startPos = 0;
		while (startPos < nRead)
		{
			int endPos = startPos;
			while (buf[endPos] != '\n' && endPos < nRead)
				++endPos;
			buf[endPos] = 0;
			if (endPos > startPos)
				LOG("stderr", ("%s", buf+startPos));
			startPos = endPos+1;
		}
	}
}

// ======================================================================

