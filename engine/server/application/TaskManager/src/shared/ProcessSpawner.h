#ifndef	_PROCESS_SPAWNER_H
#define _PROCESS_SPAWNER_H

#include <string>
#include <vector>

class ProcessSpawner
{
public:
	static uint32   execute(const std::string & commandLine, const std::vector<std::string> & parameters);
	static uint32   execute(const std::string & commandLine);
	static void     forceCore(const unsigned long pid);
	static bool     isProcessActive(uint32 pid);
	static void		kill(uint32 pid);
	static uint32	postfixCounter;
	static uint32	prefix;
private:
	ProcessSpawner();
	~ProcessSpawner();
	ProcessSpawner(const ProcessSpawner&);
	ProcessSpawner& operator= (const ProcessSpawner&);
};

//-----------------------------------------------------------------------

#endif	// _PROCESS_SPAWNER_H
