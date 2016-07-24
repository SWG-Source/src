#include "FirstTaskManager.h"
#include "sharedFoundation/FirstSharedFoundation.h"
#include "ProcessSpawner.h"
#include <map>
#include <string>
#include "TaskManager.h"

#include <stdio.h>

uint32 ProcessSpawner::prefix;
std::map<uint32, HANDLE> procById;

//-----------------------------------------------------------------------

bool tokenize (const std::string & str, std::vector<std::string> & result)
{
	size_t end_pos = 0;
	size_t start_pos = 0;

	result.clear ();

	for (;;)
	{
		if (end_pos >= str.size ())
			break;

		start_pos = str.find_first_not_of (' ', end_pos);

		if (start_pos == str.npos)
			break;

		//----------------------------------------------------------------------

		if (str [start_pos] == '\"')
		{
			 if (++start_pos >= str.size ())
				 break;
			end_pos = str.find_first_of ('\"', start_pos);
		}
		else
			end_pos = str.find_first_of (' ', start_pos);
		
		//----------------------------------------------------------------------
					
		if (start_pos == end_pos)
			break;

		if (end_pos == str.npos)
		{
			result.push_back (str.substr (start_pos));
			break;
		}
		else
			result.push_back (str.substr (start_pos, end_pos - start_pos));

		 ++start_pos;
	}
	
	return true;
}

uint32 ProcessSpawner::execute(const std::string & processName, const std::vector<std::string> & parameters)
{
	STARTUPINFO	si;
	PROCESS_INFORMATION	pi;
	char				cmd[1024] = {"\0"};
	std::string			cmdLine;

	cmdLine = processName.c_str();
	cmdLine += " ";
	std::vector<std::string>::const_iterator i;
	for(i = parameters.begin(); i != parameters.end(); ++i)
	{
		cmdLine += (*i).c_str();
		cmdLine += " ";
	}

	_snprintf(cmd, 1024, "%s.exe", processName.c_str());
//	_snprintf(cmd, 1024, "%s", processName.c_str());
	memset(&si, 0, sizeof(si));
	memset(&pi, 0, sizeof(pi));

	si.cb = sizeof(si);

	const int result = CreateProcess(cmd, const_cast<char *>(cmdLine.c_str()), NULL, NULL, false, 0, 0, 0, &si, &pi);
	UNREF (result);

#ifdef _DEBUG
	if (!result)
	{
		DWORD iErr = GetLastError();
		char * errStr = strerror(iErr);
		DEBUG_REPORT_LOG(true, ("ProcessSpawner: %s - %s\n", cmd, errStr));
	}
#endif

	procById.insert(std::pair<uint32, HANDLE>(pi.dwProcessId, pi.hProcess));
	return pi.dwProcessId;
}

//-----------------------------------------------------------------------

uint32 ProcessSpawner::execute(const std::string & cmd)
{
	std::vector<std::string> args;
	size_t firstArg = cmd.find_first_of(" ");
	std::string processName;
	if(firstArg < cmd.size())
	{
		std::string a = cmd.substr(firstArg + 1);
		tokenize(a, args);
		processName = cmd.substr(0, firstArg);
	}
	else
	{
		processName = cmd;
	}
	return execute(processName, args);
}

//-----------------------------------------------------------------------

bool ProcessSpawner::isProcessActive(uint32 pid)
{
	bool result = false;
	std::map<uint32, HANDLE>::const_iterator f = procById.find(pid);
	if(f != procById.end())
	{
		DWORD exitCode;
		GetExitCodeProcess((*f).second, &exitCode);
		result = (exitCode == STILL_ACTIVE);
	}
	return result;
}

//-----------------------------------------------------------------------

void ProcessSpawner::kill(uint32 pid)
{
	HANDLE p = OpenProcess(PROCESS_TERMINATE, false, (DWORD)pid);
	if(p)
		TerminateProcess(p, 0);
}

//-----------------------------------------------------------------------

void ProcessSpawner::forceCore(const unsigned long pid)
{
	ProcessSpawner::kill(pid);
}

//-----------------------------------------------------------------------

