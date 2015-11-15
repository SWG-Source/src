#ifndef SERVER__SYSTEMLOG_H
#define SERVER__SYSTEMLOG_H


#include <stdio.h>


class SysLog
{
	public:
		enum { LOG_EMERG, LOG_ALERT, LOG_CRIT, LOG_ERR, LOG_WARNING, LOG_NOTICE, LOG_INFO, LOG_DEBUG };
		enum { DEFAULT_LOGOPT = -1, DEFAULT_FACILITY = -1 };

		SysLog(const char * identity, bool systemOutput = true, bool localOutput = false, int logopt = DEFAULT_LOGOPT, int facility = DEFAULT_FACILITY);
		~SysLog();
		void Log(int priority, const char *message);
		void Log(int priority, const char *message, va_list argptr);
		void vLog(int priority, const char *message, ...);

		void EnableFlushOnEveryLocalWrite(bool bEnable=true) { mFlushOnEveryLocalWrite=bEnable; }

	private:
		void CheckLocalFile(unsigned currentTime);

	private:
		char		mIdentity[64];
		bool		mSystemOutput;
		bool		mLocalOutput;
		bool		mFlushOnEveryLocalWrite;
		FILE *		mLogFile;
		unsigned	mFileTimeout;
		char		mTimeStamp[64];
		unsigned	mStampTimeout;
};


#endif	//	#ifndef SERVER__SYSTEMLOG_H
