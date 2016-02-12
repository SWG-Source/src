#include "sharedFoundation/FirstSharedFoundation.h"
#include "ProcessSpawner.h"
#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include <wait.h>

#include <vector>

uint32 ProcessSpawner::prefix;
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

		if (start_pos == str.npos) //lint !e1705
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

		if (end_pos == str.npos) //lint !e1705
		{
			result.push_back (str.substr (start_pos));
			break;
		}
		else
		{
			result.push_back (str.substr (start_pos, end_pos - start_pos));
		}
		++start_pos;
	}
	
	return true;
}

// ----------------------------------------------------------------------

uint32 execute(const char * commandLine, char * const * parameters)
{
	DEBUG_REPORT_LOG(true, ("Now Attempting To Spawn %s\n", commandLine));
	pid_t p;
	// fork
	p = fork();
	if (p == 0)
	{
		// i am child
		DEBUG_REPORT_LOG(true, ("Now Spawning %s\n", commandLine));

		int result = execv(commandLine, parameters); //lint !e10 Expecting a function (huh?)
		if (result == -1)
		{
			IGNORE_RETURN(perror("execv")); //lint !e10
			_exit(0);
		}
	}
	return p; //lint !e732 Loss of sign
}

// ----------------------------------------------------------------------

void makeParameters(const std::vector<std::string> & parameters, std::vector<char *> & p)
{
 	std::vector<std::string>::const_iterator i;
	for(i = parameters.begin(); i != parameters.end(); ++i)
	{
		char * arg = new char[(*i).length() + 1];
		strcpy(arg, (*i).c_str()); //lint !e64 !e534
		p.push_back(arg);
	}
	p.push_back(nullptr);
}

// ----------------------------------------------------------------------

void freeParameters(std::vector<char *> & p)
{
	std::vector<char *>::iterator piter;
	for(piter = p.begin(); piter != p.end(); ++piter)
	{
		char * arg = (*piter);
		delete[] arg;
	}
}

// ----------------------------------------------------------------------

uint32 ProcessSpawner::execute(const std::string & commandLine, const std::vector<std::string> & parameters)
{

	std::vector<char *> p;
	makeParameters(parameters, p);
	
	std::string c = "./";
	c += commandLine;
	uint32 pid = ::execute(c.c_str(), &p[0]);

	freeParameters(p);
		
	return pid;
}

// ----------------------------------------------------------------------

uint32 ProcessSpawner::execute(const std::string & commandLine)
{
	std::vector<std::string> parameters;
	IGNORE_RETURN(tokenize(commandLine, parameters));
	
	std::vector<char *> p;
	makeParameters(parameters, p);
		
	uint32 result = ::execute(p[0], &p[0]);

	freeParameters(p);
	
	return result; 
}
// ----------------------------------------------------------------------

bool ProcessSpawner::isProcessActive(uint32 pid)
{
	pid_t result = waitpid(static_cast<pid_t>(pid), 0, WNOHANG|WUNTRACED); 
	return (result < 1);
}

// ----------------------------------------------------------------------

void ProcessSpawner::kill(uint32 pid)
{
    IGNORE_RETURN(::kill(static_cast<int>(pid), SIGKILL));
	int status;
	IGNORE_RETURN(waitpid(static_cast<int>(pid), &status, 0));
}

// ----------------------------------------------------------------------

void ProcessSpawner::forceCore(const unsigned long pid)
{
	IGNORE_RETURN(::kill(static_cast<int>(pid), SIGABRT));
}

// ----------------------------------------------------------------------
