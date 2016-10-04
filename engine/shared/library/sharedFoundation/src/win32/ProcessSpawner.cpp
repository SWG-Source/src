//
// ProcessSpawner.cpp
//
//-------------------------------------------------------------------

#include "sharedFoundation/FirstSharedFoundation.h"
#include "sharedFoundation/ProcessSpawner.h"

#include "sharedFoundation/Os.h"

#include <minmax.h>

ProcessSpawner::ProcessSpawner()
{
	m_asConsole=false;
	hProcess=0;
	hOutputRead=0;
	hInputWrite=0;
	currentLine=currentRead=readBuffer;
}


ProcessSpawner::~ProcessSpawner()
{
	if (hProcess)
	{
		CloseHandle(hProcess);
		hProcess=0;
	}
	if (hOutputRead)
	{
		CloseHandle(hOutputRead);
		hOutputRead=0;
	}
	if (hInputWrite)
	{
		CloseHandle(hInputWrite);
		hInputWrite=0;
	}
}

bool ProcessSpawner::terminate(unsigned exitCode)
{
	if (!hProcess)
	{
		return false;
	}
	return TerminateProcess(hProcess, exitCode)!=0;
}

bool ProcessSpawner::create(const char *commandLine, const char *startupFolder, bool asConsole)
{
	if (hProcess)
	{
		return false;
	}

	if (!commandLine)
	{
		return false;
	}

	if (!startupFolder)
	{
		startupFolder=Os::getProgramStartupDirectory();
	}

	m_asConsole=asConsole;

	STARTUPINFO sinfo;
	memset(&sinfo, 0, sizeof(sinfo));
	sinfo.cb=sizeof(sinfo);

	HANDLE hOutputWrite=0;
	HANDLE hErrorWrite=0;
	HANDLE hInputRead=0;

	if (asConsole)
	{
		SECURITY_ATTRIBUTES sa;
		sa.nLength=sizeof(sa);
		sa.lpSecurityDescriptor=0;
		sa.bInheritHandle=true;

		// -----------------------------------------------------

		// Create the child output pipe.
		HANDLE hOutputReadTmp;
		CreatePipe(&hOutputReadTmp,&hOutputWrite,&sa,0);

		// Create a duplicate of the output write handle for the std error
		// write handle. This is necessary in case the child application
		// closes one of its std output handles.
		DuplicateHandle(
			GetCurrentProcess(), hOutputWrite,
			GetCurrentProcess(),&hErrorWrite,
			0,
			TRUE,DUPLICATE_SAME_ACCESS
		);


		// Create the child input pipe.
		HANDLE hInputWriteTmp;
		CreatePipe(&hInputRead,&hInputWriteTmp,&sa,0);

		// Create new output read handle and the input write handles. Set
		// the Properties to FALSE. Otherwise, the child inherits the
		// properties and, as a result, non-closeable handles to the pipes
		// are created.
		DuplicateHandle(
			GetCurrentProcess(),  hOutputReadTmp,
			GetCurrentProcess(), &hOutputRead, // Address of new handle.
			0, FALSE, // Make it uninheritable.
			DUPLICATE_SAME_ACCESS
		);

		DuplicateHandle(
			GetCurrentProcess(),  hInputWriteTmp,
			GetCurrentProcess(), &hInputWrite, // Address of new handle.
			0,FALSE, // Make it uninheritable.
			DUPLICATE_SAME_ACCESS
		);


		// Close inheritable copies of the handles you do not want to be
		// inherited.
		CloseHandle(hOutputReadTmp); hOutputReadTmp=0;
		CloseHandle(hInputWriteTmp); hInputWriteTmp=0;

		sinfo.dwFlags|=(STARTF_USESTDHANDLES | STARTF_USESHOWWINDOW);
		sinfo.hStdError=hErrorWrite;
		sinfo.hStdInput=hInputRead;
		sinfo.hStdOutput=hOutputWrite;
		sinfo.wShowWindow = SW_HIDE;
	}

	PROCESS_INFORMATION pinfo;
	BOOL result = CreateProcess(
		0,
		(char *)commandLine,
		0,
		0,
		TRUE,
		CREATE_NEW_CONSOLE,
		0,
		startupFolder,
		&sinfo,
		&pinfo
	);

	if (asConsole)
	{
		// Close pipe handles (do not continue to modify the parent).
		// You need to make sure that no handles to the write end of the
		// output pipe are maintained in this process or else the pipe will
		// not close when the child process exits and the ReadFile will hang.
		CloseHandle(hOutputWrite); hOutputWrite=0;
		CloseHandle(hErrorWrite); hErrorWrite=0;
		CloseHandle(hInputRead); hInputRead=0;
	}

	if (result)
	{
		CloseHandle(pinfo.hThread);
		hProcess=pinfo.hProcess;
		return true;
	}
	else
	{
		// Failed to launch turf
		hProcess=0;
		return false;
	}
}

bool ProcessSpawner::isFinished(unsigned waitTime)
{
	if (!hProcess)
	{
		return true;
	}

	DWORD waitResult = WaitForSingleObject(hProcess, waitTime);

	return waitResult==WAIT_OBJECT_0;
}

bool ProcessSpawner::getExitCode(unsigned &o_code)
{
	if (!hProcess)
	{
		return false;
	}

	DWORD exitCode;
	BOOL result = GetExitCodeProcess(hProcess, &exitCode);
	if (result)
	{
		o_code=exitCode;
		return true;
	}
	else
	{
		return false;
	}
}

bool ProcessSpawner::_returnExistingLine(char *buffer, const int bufferSize)
{
	const char *const bufferStop = buffer + bufferSize;
	char *iter = currentLine;
	while (iter!=currentRead)
	{
		if (buffer==bufferStop)
		{
			currentLine=iter;
			return true;
		}

		if (*iter=='\n')
		{
			*buffer++=0;
			_stepIter(iter);
			currentLine=iter;
			return true;
		}
		else
		{
			*buffer++=*iter;
			_stepIter(iter);
		}
	}

	return false;
}

bool ProcessSpawner::getOutputString(char *buffer, int bufferSize)
{
	if (_returnExistingLine(buffer, bufferSize))
	{
		return true;
	}

	// ----------------------------------------------

	if (!hOutputRead)
	{
		return false;
	}

	DWORD dwAvail = 0;
	if (!::PeekNamedPipe(hOutputRead, NULL, 0, NULL, &dwAvail, NULL))
	{
		// ERROR
		return false;
	}

	if (!dwAvail)
	{
		return false;
	}

	DWORD dwRead;

	if (currentRead >= currentLine)
	{
		const unsigned bufferAvailable = sizeof(readBuffer) - (currentRead - readBuffer);
		unsigned toRead = dwAvail;
		if (toRead > bufferAvailable)
		{
			toRead=bufferAvailable;
		}

		dwRead=0;
		if (!::ReadFile(hOutputRead, currentRead, min(bufferAvailable, dwAvail), &dwRead, NULL) || !dwRead)
		{
			return false;
		}
		dwAvail-=dwRead;
		currentRead+=dwRead;
		if (currentRead==readBuffer+sizeof(readBuffer))
		{
			currentRead=readBuffer;
		}
	}

	if (dwAvail>0)
	{
		const unsigned bufferAvailable = currentLine - currentRead - 1;
		if (bufferAvailable)
		{
			unsigned toRead = dwAvail;
			if (toRead > bufferAvailable)
			{
				toRead=bufferAvailable;
			}

			dwRead=0;
			if (!::ReadFile(hOutputRead, currentRead, min(bufferAvailable, dwAvail), &dwRead, NULL) || !dwRead)
			{
				return false;
			}
			currentRead+=dwRead;

			DEBUG_FATAL(currentRead>=currentLine, (""));
		}
	}

	return _returnExistingLine(buffer, bufferSize);
}
