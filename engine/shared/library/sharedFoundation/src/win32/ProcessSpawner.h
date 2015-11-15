#ifndef INCLUDED_ProcessSpawner_H
#define INCLUDED_ProcessSpawner_H

class ProcessSpawner
{
public:

	ProcessSpawner();
	~ProcessSpawner();

	bool create(const char *commandLine, const char *startupFolder=0, bool asConsole=true);

	bool terminate(unsigned exitCode=0);
	bool isFinished(unsigned waitTime=0);
	bool getExitCode(unsigned &o_code);

	bool getOutputString(char *buffer, int bufferSize);

protected:

	bool _returnExistingLine(char *buffer, int bufferSize);

	void _stepIter(char *&i)
	{
		if (i==readBuffer+sizeof(readBuffer)-1)
		{
			i=readBuffer;
		}
		else
		{
			i++;
		}
	}

	HANDLE hProcess;
	HANDLE hOutputRead, hInputWrite;
	bool m_asConsole;
	char *currentLine, *currentRead, readBuffer[4096];
};

#endif


