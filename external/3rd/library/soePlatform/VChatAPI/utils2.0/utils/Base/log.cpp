
#include "log.h"
#if defined (_MT)
#include "thread.h"
#endif
#include <sys/stat.h>
#include "types.h"

#ifdef WIN32
#include <time.h>
#include <stdarg.h>
#include <io.h>
#include <direct.h>
#include <stdio.h>
#include <errno.h>
#include <fcntl.h>
#else
#include <errno.h>
#include <assert.h>
#include <sys/errno.h>
#include <pthread.h>
#include <resolv.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#endif


#ifdef WIN32
#define snprintf _snprintf
#endif


using namespace std;

#ifdef EXTERNAL_DISTRO
namespace NAMESPACE 
{

#endif
namespace soe
{

const long kDateLenAllowance = 256;

#ifdef WIN32
const char file_sep = '\\';

#define LOGGER_GET_CURR_TIME(__tm_struct__, __time_t_var__)						\
	memcpy(&__tm_struct__, localtime(&__time_t_var__), sizeof(__tm_struct__))

#define LOGGER_MAKE_DIR(__dir_name__, __mode__)	mkdir(__dir_name__)

#define vsnprintf _vsnprintf

#else  // ifndef WIN32
const char file_sep = '/';

const char *syslogLevels[] = {
	"EMERG", "ALERT", "CRIT", "ERR", "WARNING", "NOTICE", "INFO", "DEBUG" };

const int numSysLogLevels = sizeof(syslogLevels) / sizeof(char *);

#define LOGGER_GET_CURR_TIME(__tm_struct__, __time_t_var__)						\
	localtime_r(&__time_t_var__, &__tm_struct__)

#define LOGGER_MAKE_DIR(__dir_name__, __mode__)	mkdir(__dir_name__, __mode__)

#endif // WIN32

#ifdef _MT
#define LOGGER_LOCK			ScopeLock logLock(rLock);
#else // ifndef _MT
#define LOGGER_LOCK
#endif // _MT

Logger::Logger(const char *prefix, 
			   int level, 
			   unsigned size, 
			   bool rollDate)
: m_defaultLevel(level), 
  m_defaultSize(size), 
  m_dirPrefix(prefix), 
  m_rollDate(rollDate),
  m_unlimitedLineSize(false)
{
	m_logType = eUseLocalFile;
	LoggerInit("UnNamedServer");
}

Logger::Logger(const char *programName, 
			   const ELogType logType, 
			   const char *prefix, 
			   int level, 
			   unsigned size, 
			   bool rollDate)
: m_defaultLevel(level), 
  m_defaultSize(size), 
  m_dirPrefix(prefix), 
  m_rollDate(rollDate), 
  m_logType(logType),
  m_unlimitedLineSize(false)
{
	LoggerInit(programName);
}


void Logger::LoggerInit(const char *programName)
{
	char buf[1024];
	FILE *logDir = nullptr;
	
	if (0 != (m_logType & eUseLocalFile))
	{
		logDir = fopen(m_dirPrefix.c_str(), "r+");
		if(errno == ENOENT)
		{
			cmkdir(m_dirPrefix.c_str(), 0755);
		}
		else if(logDir != nullptr)
		{
			fclose(logDir);
		}
		
		tm now;
		time_t t = time(nullptr);

		LOGGER_GET_CURR_TIME(now, t);

		memcpy(&m_lastDateTime, &now, sizeof(tm));
		if(m_rollDate)
		{
			snprintf(buf, sizeof(buf), "%s%c%2.2d-%2.2d-%2.2d", m_dirPrefix.c_str(), file_sep, (now.tm_mon + 1), now.tm_mday, (now.tm_year % 100));
		}
		else
		{
			snprintf(buf, sizeof(buf), "%s", m_dirPrefix.c_str());
		}
		logDir = fopen(buf, "r+");
		if(errno == ENOENT)
		{
			cmkdir(buf, 0755);
		}
		else if(logDir != nullptr)
		{
			fclose(logDir);
		}
		m_logPrefix = buf;
		m_logRolloverSize = 0x80000000; // Default: 2 GB upper limit on file size
	}

	// need to do this because syslog does not copy the buffer; it copies the *pointer*.
	setProgramName(programName);

#ifndef WIN32
	// Open connection to syslog
	if (0 != (m_logType & eUseSysLog)) {
		openlog(m_programName, LOG_NDELAY | LOG_PID, LOG_LOCAL5);
	}
#endif // !WIN32

}

Logger::~Logger()
{
	map<unsigned, LogInfo *>::iterator iter;
	for(iter = m_logTable.begin(); iter != m_logTable.end(); iter++)
	{
		logWithSys((*iter).first, LOG_INFO, LOG_FILEONLY, "---=== Log Stopped ===---");
		fflush((*iter).second->file);
		fclose((*iter).second->file);
		delete((*iter).second);
	
	}
	m_logTable.clear();
	
#ifndef WIN32
	// close connection to syslog
	if (0 != (m_logType & eUseSysLog))
		closelog();
#endif // !WIN32
}

void Logger::setProgramName(const char *programName)
{
	if (programName)
	{
		strncpy(m_programName, programName, sizeof(m_programName));
		m_programName[sizeof(m_programName) - 1] = '\0';
	}
}

void Logger::flush(unsigned logenum)
{
	map<unsigned, LogInfo *>::iterator iter = m_logTable.find(logenum);

	if(iter != m_logTable.end())
	{
		LogInfo *info = (*iter).second;
		fflush(info->file);
		info->used = 0;
	}
}

void Logger::flushAll()
{
	if (0 != (m_logType & eUseLocalFile)) 
	{
		map<unsigned, LogInfo *>::iterator iter;

		for(iter = m_logTable.begin(); iter != m_logTable.end(); iter++)
		{
			LogInfo *info = (*iter).second;
			fflush(info->file);
			info->used = 0;
		}
	}
}

void Logger::addLog(const char *id, unsigned logenum, int level, unsigned size)
{
	if (0 != (m_logType & eUseLocalFile)) 
	{
		LogInfo *newLog = new LogInfo;
		newLog->filename = m_logPrefix + file_sep + id + ".log";
		newLog->name = id;	
		newLog->used = 0;
		newLog->max = size;
		newLog->last = 0;
		newLog->level = level;

		m_logTable.insert(pair<unsigned, LogInfo *>(logenum, newLog));
		
		if(strcmp("stdout", id) == 0)
		{
			newLog->file = stdout;
		}
		else if(strcmp("stderr", id) == 0)
		{
			newLog->file = stderr;
		}
		else
		{
			newLog->file = fopen(newLog->filename.c_str(), "a+");
		}
		logWithSys(logenum, LOG_INFO, LOG_FILEONLY, "---=== Log Started ===---");
	}
}

void Logger::addLog(const char *id, unsigned logenum)
{
	if (0 != (m_logType & eUseLocalFile)) 
	{
		LogInfo *newLog = new LogInfo;
		newLog->filename = m_logPrefix + file_sep + id + ".log";
		newLog->name = id;
		newLog->used = 0;
		newLog->max = m_defaultSize;
		newLog->last = 0;
		newLog->level = m_defaultLevel;

		m_logTable.insert(pair<unsigned, LogInfo *>(logenum, newLog));

		if(strcmp("stdout", id) == 0)
		{
			newLog->file = stdout;
		}
		else if(strcmp("stderr", id) == 0)
		{
			newLog->file = stderr;
		}
		else
		{
			newLog->file = fopen(newLog->filename.c_str(), "a+");
		}
		logWithSys(logenum, LOG_INFO, LOG_FILEONLY, "---===Log Started ===---");
	}
}

void Logger::updateLog(unsigned logenum, int level, unsigned size)
{
	if (0 != (m_logType & eUseLocalFile)) 
	{
		map<unsigned, LogInfo *>::iterator iter = m_logTable.find(logenum);

		if(iter != m_logTable.end())
		{
			(*iter).second->level = level;
			(*iter).second->max = size;
		}
	}
}

void Logger::removeLog(unsigned logenum)
{
	if (0 != (m_logType & eUseLocalFile)) 
	{
		map<unsigned, LogInfo *>::iterator iter = m_logTable.find(logenum);

		if(iter != m_logTable.end())
		{
#ifdef WIN32
			logWithSys((*iter).first, LOG_INFO, LOG_FILEONLY, "---=== Log Stopped ===---");
#else // ifndef WIN32
			logWithSys((*iter).first, LOG_INFO, LOG_ALWAYS, "---=== Log Stopped ===---");
#endif // WIN32
			fflush((*iter).second->file);
			fclose((*iter).second->file);
			delete((*iter).second);
		}
	}
}





void Logger::logSimple(unsigned logenum, int level, const char *message)
{
	map<unsigned, LogInfo *>::iterator iter = m_logTable.find(logenum);
	
	if(iter == m_logTable.end())
	{
		return;
	}
	time_t t = time(nullptr);
	LogInfo *info = (*iter).second;
	if(level >= info->level)
	{
		return;
	}
#ifndef WIN32
	if(level == LOG_FILEONLY && ((info->file  == stderr) || (info->file == stdout)))
	{
		return;
	}
#endif // !WIN32
	if(info->last != t)
	{
		memcpy(&info->ts, localtime(&t), sizeof(tm));
		info->last = t;
	}
											
	if(m_rollDate && info->ts.tm_mday != m_lastDateTime.tm_mday)
	{
		memcpy(&m_lastDateTime, &info->ts, sizeof(tm));
		rollDate(t);
	}
	if(iter != m_logTable.end())
	{
		if(info->max > 0)
		{
			info->used += fprintf(info->file, "[%2.2d/%2.2d/%2.2d %2.2d:%2.2d:%2.2d] %s", (info->ts.tm_mon + 1), info->ts.tm_mday, (info->ts.tm_year % 100), info->ts.tm_hour, info->ts.tm_min, info->ts.tm_sec, message);
			info->used += fputc('\n', info->file);
			if(info->used > info->max)
			{
				fflush(info->file);
				info->used = 0;
			}
		}
		else
		{
			fprintf(info->file, "[%2.2d/%2.2d/%2.2d %2.2d:%2.2d:%2.2d] %s", (info->ts.tm_mon + 1), info->ts.tm_mday, (info->ts.tm_year % 100), info->ts.tm_hour, info->ts.tm_min, info->ts.tm_sec, message);
			fputc('\n', info->file);
			fflush(info->file);
		}
	}
}











void Logger::logSimpleWithSys(unsigned logenum, unsigned priority, int level, const char *message)
{

#ifndef WIN32
	if (0 != (m_logType & eUseSysLog))
		syslog(priority, "%s: %s", syslogLevels[priority % numSysLogLevels], message);
#endif // !WIN32

	if (0 != (m_logType & eUseLocalFile))
	{
		map<unsigned, LogInfo *>::iterator iter = m_logTable.find(logenum);
		
		if(iter == m_logTable.end())
		{
			return;
		}
		time_t t = time(nullptr);
		LogInfo *info = (*iter).second;

		if(level >= info->level)
		{
			return;
		}

#ifndef WIN32
		if(level == LOG_FILEONLY && ((info->file  == stderr) || (info->file == stdout)))
		{
			return;
		}
#endif // !WIN32

		if(info->last != t)
		{
			memcpy(&info->ts, localtime(&t), sizeof(tm));
			info->last = t;
		}
												
		if(m_rollDate && info->ts.tm_mday != m_lastDateTime.tm_mday)
		{
			memcpy(&m_lastDateTime, &info->ts, sizeof(tm));
			rollDate(t);
		}
		if(iter != m_logTable.end())
		{
			if (m_logRolloverSize> 0)
				checkLogRoll(info, (unsigned long)::strlen(message) + kDateLenAllowance);

			if(info->max > 0)
			{
				info->used += fprintf(info->file, "[%2.2d/%2.2d/%2.2d %2.2d:%2.2d:%2.2d] %s", (info->ts.tm_mon + 1), info->ts.tm_mday, (info->ts.tm_year % 100), info->ts.tm_hour, info->ts.tm_min, info->ts.tm_sec, message);
				info->used += fputc('\n', info->file);
				if(info->used > info->max)
				{
					fflush(info->file);
					info->used = 0;
				}
			}
			else
			{
				fprintf(info->file, "[%2.2d/%2.2d/%2.2d %2.2d:%2.2d:%2.2d] %s", (info->ts.tm_mon + 1), info->ts.tm_mday, (info->ts.tm_year % 100), info->ts.tm_hour, info->ts.tm_min, info->ts.tm_sec, message);
				fputc('\n', info->file);
				fflush(info->file);
			}
		}
	}
}

void Logger::log(unsigned logenum, int level, const char *message, ...)
{
	va_list varg;

	va_start(varg, message);
	vlogWithSys(logenum, LOG_NOTICE, level, message, varg);
	va_end(varg);
}


void Logger::logWithSys(unsigned logenum, unsigned priority, int level, const char *message, ...)
{
	char buf[2048];
	va_list varg;
	va_start(varg, message);
	vsnprintf(buf, 2047, message, varg);
	va_end(varg);
	buf[2047] = 0;

#ifndef WIN32	
	if (0 != (m_logType & eUseSysLog))
		syslog(priority, "%s: %s", syslogLevels[priority % numSysLogLevels], buf);
#endif // !WIN32

	if (0 != (m_logType & eUseLocalFile)) 
	{
		map<unsigned, LogInfo *>::iterator iter = m_logTable.find(logenum);
		if(iter == m_logTable.end())
		{
					return;
		}
		time_t t = time(nullptr);
		LogInfo *info = (*iter).second;

		if(level >= info->level)
		{
			return;
		}
		if(level == LOG_FILEONLY && ((info->file  == stderr) || (info->file == stdout)))
		{
			return;
		}
		if(info->last != t)
		{
			LOGGER_GET_CURR_TIME(info->ts, t);

			info->last = t;
		}
		if(m_rollDate && info->ts.tm_mday != m_lastDateTime.tm_mday)
		{
			memcpy(&m_lastDateTime, &info->ts, sizeof(tm));
			rollDate(t);
		}

		if(iter != m_logTable.end())
		{			
			if (m_logRolloverSize> 0)
				checkLogRoll(info, (unsigned long)::strlen(buf) + kDateLenAllowance);

			if(info->max > 0)
			{
				if (m_unlimitedLineSize) {
					info->used += fprintf(info->file, "[%2.2d/%2.2d/%2.2d %2.2d:%2.2d:%2.2d] ", (info->ts.tm_mon + 1), info->ts.tm_mday, (info->ts.tm_year % 100), info->ts.tm_hour, info->ts.tm_min, info->ts.tm_sec);
					va_start(varg, message);
					info->used += vfprintf(info->file, message, varg);
					va_end(varg);
				} else {
					info->used += fprintf(info->file, "[%2.2d/%2.2d/%2.2d %2.2d:%2.2d:%2.2d] %s", (info->ts.tm_mon + 1), info->ts.tm_mday, (info->ts.tm_year % 100), info->ts.tm_hour, info->ts.tm_min, info->ts.tm_sec, buf);
				}
				info->used += fputc('\n', info->file);
				if(info->used > info->max)
				{
					fflush(info->file);
					info->used = 0;
				}
			}
			else
			{
				if (m_unlimitedLineSize) {
					info->used += fprintf(info->file, "[%2.2d/%2.2d/%2.2d %2.2d:%2.2d:%2.2d] ", (info->ts.tm_mon + 1), info->ts.tm_mday, (info->ts.tm_year % 100), info->ts.tm_hour, info->ts.tm_min, info->ts.tm_sec);
					va_start(varg, message);
					info->used += vfprintf(info->file, message, varg);
					va_end(varg);
				} else {
					info->used += fprintf(info->file, "[%2.2d/%2.2d/%2.2d %2.2d:%2.2d:%2.2d] %s", (info->ts.tm_mon + 1), info->ts.tm_mday, (info->ts.tm_year % 100), info->ts.tm_hour, info->ts.tm_min, info->ts.tm_sec, buf);
				}
				fputc('\n', info->file);
				fflush(info->file);
			}
		}
	}
}

void Logger::vlogWithSys(unsigned logenum, unsigned priority, int level, const char *message, va_list argptr)
{
	char buf[2048];
	vsnprintf(buf, 2047, message, argptr);
	buf[2047] = 0;

#ifndef WIN32	
	if (0 != (m_logType & eUseSysLog))
		syslog(priority, "%s: %s", syslogLevels[priority % numSysLogLevels], buf);
#endif // !WIN32

	if (0 != (m_logType & eUseLocalFile)) 
	{
		map<unsigned, LogInfo *>::iterator iter = m_logTable.find(logenum);
		if(iter == m_logTable.end())
		{
			return;
		}
		time_t t = time(nullptr);
		LogInfo *info = (*iter).second;

		if(level >= info->level)
		{
			return;
		}
		if(level == LOG_FILEONLY && ((info->file  == stderr) || (info->file == stdout)))
		{
			return;
		}
		if(info->last != t)
		{
			LOGGER_GET_CURR_TIME(info->ts, t);

			info->last = t;
		}
		if(m_rollDate && info->ts.tm_mday != m_lastDateTime.tm_mday)
		{
			memcpy(&m_lastDateTime, &info->ts, sizeof(tm));
			rollDate(t);
		}

		if(iter != m_logTable.end())
		{			
			if (m_logRolloverSize> 0)
				checkLogRoll(info, (unsigned long)::strlen(buf) + kDateLenAllowance);

			if(info->max > 0)
			{
				if (m_unlimitedLineSize) {
					info->used += fprintf(info->file, "[%2.2d/%2.2d/%2.2d %2.2d:%2.2d:%2.2d] ", (info->ts.tm_mon + 1), info->ts.tm_mday, (info->ts.tm_year % 100), info->ts.tm_hour, info->ts.tm_min, info->ts.tm_sec);
					info->used += vfprintf(info->file, message, argptr);
				} else {
					info->used += fprintf(info->file, "[%2.2d/%2.2d/%2.2d %2.2d:%2.2d:%2.2d] %s", (info->ts.tm_mon + 1), info->ts.tm_mday, (info->ts.tm_year % 100), info->ts.tm_hour, info->ts.tm_min, info->ts.tm_sec, buf);
				}
				info->used += fputc('\n', info->file);
				if(info->used > info->max)
				{
					fflush(info->file);
					info->used = 0;
				}
			}
			else
			{
				if (m_unlimitedLineSize) {
					info->used += fprintf(info->file, "[%2.2d/%2.2d/%2.2d %2.2d:%2.2d:%2.2d] ", (info->ts.tm_mon + 1), info->ts.tm_mday, (info->ts.tm_year % 100), info->ts.tm_hour, info->ts.tm_min, info->ts.tm_sec);
					info->used += vfprintf(info->file, message, argptr);
				} else {
					info->used += fprintf(info->file, "[%2.2d/%2.2d/%2.2d %2.2d:%2.2d:%2.2d] %s", (info->ts.tm_mon + 1), info->ts.tm_mday, (info->ts.tm_year % 100), info->ts.tm_hour, info->ts.tm_min, info->ts.tm_sec, buf);
				}
				fputc('\n', info->file);
				fflush(info->file);
			}
		}
	}
}

void Logger::vlog(unsigned logenum, int level, const char *message, va_list argptr)
{
	vlogWithSys(logenum, LOG_NOTICE, level, message, argptr);
}

// void Logger::vlogWithSys(unsigned logenum, unsigned priority, int level, const char *message, va_list argptr)

void Logger::rollDate(time_t t)
{
	char buf[80];
	FILE *logDir = nullptr;
	
	logDir = fopen(m_dirPrefix.c_str(), "r+");
	if(errno == ENOENT)
	{
		cmkdir(m_dirPrefix.c_str(), 0755);
	}
	else if(logDir != nullptr)
	{
		fclose(logDir);
	}

	tm now;

	LOGGER_GET_CURR_TIME(now, t);

	snprintf(buf, sizeof(buf), "%s%c%2.2d-%2.2d-%2.2d", m_dirPrefix.c_str(), file_sep, (now.tm_mon + 1), now.tm_mday, (now.tm_year % 100));
	logDir = fopen(buf, "r+");

	if(errno == ENOENT)
	{
		cmkdir(buf, 0755);
	}
	else if(logDir != nullptr)
	{
		fclose(logDir);
	}
	m_logPrefix = buf;

	map<unsigned, LogInfo *>::iterator iter;
	for(iter = m_logTable.begin(); iter != m_logTable.end(); iter++)
	{
		(*iter).second->filename = m_logPrefix + file_sep + (*iter).second->name.c_str() + ".log";
		fflush((*iter).second->file);	
		fclose((*iter).second->file);
		(*iter).second->file = fopen((*iter).second->filename.c_str(), "a+");
		if ((*iter).second->file == 0)
			abort();
		memcpy(&((*iter).second->ts), &now, sizeof(tm));
	}
}

// mkdir function that creates intermediate directories
void Logger::cmkdir(const char *dir, int mode)
{
	char dirbuf[128];
	strncpy(dirbuf, dir, 127);
	dirbuf[127] = 0;
	char *j = dirbuf, *i = dirbuf;
	int handle;

	while(*i)
	{
		if(*i == file_sep)
		{
			(*i) = 0;
#ifdef WIN32
			handle = open(j, O_EXCL);

			if((handle > 0) || (errno != EISDIR && errno != ENOENT && errno != EACCES))
			{
				perror("Logger::cmkdir():");
				abort();
			}
#else // ifndef WIN32
//			handle = open(j, O_EXCL);		// Ben's original code
//			if((handle > 0) || (errno != EISDIR && errno != ENOENT))
//			{
//				perror("Logger::cmkdir():");
//				abort();
//			}

//			This doesnt work under Linux, it returns a valid handle
//			Instead: see if file exists. If it doesnt, create directory ok
//			If it exists, do  stat to see if it is a dir.
//			If it is a dir, then ok, create the directory.
//			If it is a file, error
//			ging 9-16-2002

			handle = open(j, O_RDONLY);
			if (handle > 0)
			{
				struct stat stat_buffer;
				int ret = fstat(handle,&stat_buffer);
				if ((ret == -1) || ((stat_buffer.st_mode | S_IFDIR) == 0))
				{
					perror("Logger::cmkdir():");
					abort();
				}
			}
#endif // WIN32
			LOGGER_MAKE_DIR(j, mode);
			close(handle);
			(*i) = file_sep;
		}
		else if(*(i + 1) == 0)
		{
			LOGGER_MAKE_DIR(j, mode);
		}
		i++;
	}
}

void Logger::rollLog(LogInfo *logInfo)
{
	std::string newLogName;
	char timeStampBuffer[256];
	time_t t = time(nullptr);
	tm now;
	int r;
	int nTries = 10;

	fflush(logInfo->file);	
	fclose(logInfo->file);
	
	do
	{
		LOGGER_GET_CURR_TIME(now, t);
		snprintf(timeStampBuffer, sizeof(timeStampBuffer), "-%4.4d-%2.2d-%2.2d-%2.2d-%2.2d-%2.2d.log", 
			(now.tm_year + 1900), (now.tm_mon + 1), now.tm_mday, now.tm_hour, now.tm_min, now.tm_sec);
		newLogName = logInfo->filename.substr(0, logInfo->filename.length() - 4) + timeStampBuffer;
		r = rename(logInfo->filename.c_str(), newLogName.c_str());
		nTries--;
		t++;
	} 
	while ((r != 0) && (nTries > 0));
	
	logInfo->file = fopen(logInfo->filename.c_str(), "a+");
	memcpy(&(logInfo->ts), &now, sizeof(tm));
}

void Logger::checkLogRoll(LogInfo *logInfo, unsigned long lenToAdd)
{
	unsigned long logSize = ftell(logInfo->file);

	if ((logSize + lenToAdd) > m_logRolloverSize) 
	{
		rollLog(logInfo);
	}
}

};
#ifdef EXTERNAL_DISTRO
};
#endif

