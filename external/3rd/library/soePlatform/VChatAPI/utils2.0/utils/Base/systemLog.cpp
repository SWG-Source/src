#ifdef WIN32
#pragma warning (disable: 4786)
#endif


#include <time.h>
#include <string.h>
#include <stdarg.h>
#include "systemLog.h"
#include "profile.h"

#ifdef linux
#include <syslog.h>
#endif

const char * SysLogPriority[] = 
{
	"EMERG  ",
	"ALERT  ",
	"CRIT   ",
	"ERR    ",
	"WARNING",
	"NOTICE ",
	"INFO   ",
	"DEBUG  "
};

SysLog::SysLog(const char * identity, bool systemOutput, bool localOutput, int logopt, int facility) :
	mIdentity(),
	mSystemOutput(systemOutput),
	mLocalOutput(localOutput),
	mLogFile(0),
	mFileTimeout(0),
	mTimeStamp(),
	mStampTimeout(0),
	mFlushOnEveryLocalWrite(false)
{
	Profile profile("SysLog::SysLog()");
	strncpy((char *)mIdentity, identity, 64); mIdentity[63] = 0;
	mTimeStamp[0] = 0;
#ifdef linux
	if (logopt == DEFAULT_LOGOPT)
	{
		logopt = LOG_NDELAY|LOG_PID;
	}
	if (facility == DEFAULT_FACILITY)
	{
		facility = LOG_LOCAL5;
	}
	openlog(mIdentity, logopt, facility);
#endif
}

SysLog::~SysLog()
{
	Profile profile("SysLog::~SysLog()");
#ifdef linux
	closelog();
#endif
	if (mLogFile)
	{
		fprintf(mLogFile, "SysLog closed\n");
		fclose(mLogFile);
	}
}

void SysLog::CheckLocalFile(unsigned currentTime)
{
	Profile profile("SysLog::CheckLocalFile()");
	if (mLogFile && mFileTimeout < currentTime)
	{
		fprintf(mLogFile, "SysLog closed (continue in new file)\n");
		fclose(mLogFile);
		mLogFile = 0;
	}
	if (!mLogFile)
	{
		char filename[64];
		struct tm timeStruct = *localtime((time_t *)&currentTime);
		strftime(filename, 64, "log-%b%d-%H.txt", &timeStruct);
		timeStruct.tm_sec = 59;
		timeStruct.tm_min = 59;
		//timeStruct.tm_hour = 23;
		mFileTimeout = mktime(&timeStruct);
		mLogFile = fopen(filename, "a+t");
		if (!mLogFile)
		{
			mLocalOutput = false;
		}
		else
		{
			fprintf(mLogFile, "\nSysLog opened\n");
		}
	}
}


//	4975 ns
void SysLog::Log(int priority, const char *message)
{
	Profile profile("SysLog::Log()");
	if (priority > LOG_DEBUG)
		priority = LOG_DEBUG;
#ifdef linux
	if (mSystemOutput)
	{
		Profile profile("syslog()");
		syslog(priority, "%s : %s", SysLogPriority[priority], message);
	}
#endif
	if (mLocalOutput)
	{
		unsigned currentTime = (unsigned)time(0);
		CheckLocalFile(currentTime);
		if (mLogFile)
		{
			if (mStampTimeout < currentTime)
			{
				struct tm timeStruct = *localtime((time_t *)&currentTime);
				strftime(mTimeStamp, 64, "%b %d %H:%M:%S", &timeStruct);
				mStampTimeout = currentTime;
			}
			Profile profile("fprintf()");
			fprintf(mLogFile, "%s %s: %s : %s\n", mTimeStamp, mIdentity, SysLogPriority[priority], message);
			if (mFlushOnEveryLocalWrite)
				fflush(mLogFile);
		}
	}
}

void SysLog::Log(int priority, const char *message, va_list args)
{
	Profile profile("SysLog::Log(...)");
	if (priority > LOG_DEBUG)
		priority = LOG_DEBUG;
#ifdef linux
	if (mSystemOutput)
	{
		char buffer[10240];
		snprintf(buffer, 10240, "%s : %s", SysLogPriority[priority], message);
		Profile profile("vsyslog()");
		vsyslog(priority, buffer, args);
	}
#endif
	if (mLocalOutput)
	{
		unsigned currentTime = (unsigned)time(0);
		CheckLocalFile(currentTime);
		if (mLogFile)
		{
			if (mStampTimeout < currentTime)
			{
				struct tm timeStruct = *localtime((time_t *)&currentTime);
				strftime(mTimeStamp, 64, "%b %d %H:%M:%S", &timeStruct);
				mStampTimeout = currentTime;
			}
			Profile profile("fprintf()");
			fprintf(mLogFile, "%s %s: %s : ", mTimeStamp, mIdentity, SysLogPriority[priority]);
			vfprintf(mLogFile, message, args);
			fprintf(mLogFile, "\n");
			if (mFlushOnEveryLocalWrite)
				fflush(mLogFile);
		}
	}
}

void SysLog::vLog(int priority, const char *message, ...)
{
	Profile profile("SysLog::vLog()");
	va_list args;
	va_start(args, message);
	Log(priority, message, args);
	va_end(args);
}
