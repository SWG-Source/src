#include "../Logger.h"
#include "Mutex.h"
#include <sys/stat.h>

using namespace std;

#ifdef EXTERNAL_DISTRO
namespace NAMESPACE 
{

#endif
namespace Base
{

const char file_sep = '/';

Logger::Logger(const char *prefix, int level, unsigned size, bool rollDate)
: m_defaultLevel(level), m_defaultSize(size), m_dirPrefix(prefix), m_rollDate(rollDate)
{
	char buf[1024];
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
	time_t t = time(nullptr);
	localtime_r(&t, &now);

	memcpy(&m_lastDateTime, &now, sizeof(tm));
	if(m_rollDate)
	{
		snprintf(buf, 1024, "%s%c%2.2d-%2.2d-%2.2d", m_dirPrefix.c_str(), file_sep, (now.tm_mon + 1), now.tm_mday, (now.tm_year % 100));
	}
	else
	{
		snprintf(buf, 1024, "%s", m_dirPrefix.c_str());
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
}

Logger::~Logger()
{
	map<unsigned, LogInfo *>::iterator iter;
	for(iter = m_logTable.begin(); iter != m_logTable.end(); iter++)
	{
		log((*iter).first, LOG_FILEONLY, "---=== Log Stopped ===---");
		fflush((*iter).second->file);
		fclose((*iter).second->file);
		delete((*iter).second);
	
	}
	m_logTable.clear();
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
	map<unsigned, LogInfo *>::iterator iter;

	for(iter = m_logTable.begin(); iter != m_logTable.end(); iter++)
	{
		LogInfo *info = (*iter).second;
		fflush(info->file);
		info->used = 0;
	}
}

void Logger::addLog(const char *id, unsigned logenum, int level, unsigned size)
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
	log(logenum, LOG_FILEONLY, "---=== Log Started ===---");
}

void Logger::addLog(const char *id, unsigned logenum)
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
	log(logenum, LOG_FILEONLY, "---===Log Started ===---");
}

void Logger::updateLog(unsigned logenum, int level, unsigned size)
{
	map<unsigned, LogInfo *>::iterator iter = m_logTable.find(logenum);

	if(iter != m_logTable.end())
	{
		(*iter).second->level = level;
		(*iter).second->max = size;
	}
}

void Logger::removeLog(unsigned logenum)
{
	map<unsigned, LogInfo *>::iterator iter = m_logTable.find(logenum);

	if(iter != m_logTable.end())
	{
		log((*iter).first, LOG_ALWAYS, "---=== Log Stopped ===---");
		fflush((*iter).second->file);
		fclose((*iter).second->file);
		delete((*iter).second);
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
	if(level == LOG_FILEONLY && ((info->file  == stderr) || (info->file == stdout)))
	{
		return;
	}
	if(info->last != t)
	{
		memcpy(&info->ts, localtime(&t), sizeof(tm));
		info->last = t;
	}
											
	if(m_rollDate && info->ts.tm_mday != m_lastDateTime.tm_mday)
	{
#if defined(_REENTRANT)
		rLock.Lock();
#endif
		if(info->ts.tm_mday != m_lastDateTime.tm_mday)
		{
			memcpy(&m_lastDateTime, &info->ts, sizeof(tm));
			rollDate(t);
		}
#if defined(_REENTRANT)
		rLock.Unlock();
#endif
	}
	if(iter != m_logTable.end())
	{
		if(info->max > 0)
		{
			int tmp = fprintf(info->file, "[%2.2d/%2.2d/%2.2d %2.2d:%2.2d:%2.2d] %s\n", (info->ts.tm_mon + 1), info->ts.tm_mday, (info->ts.tm_year % 100), info->ts.tm_hour, info->ts.tm_min, info->ts.tm_sec, message);
			info->used += tmp;
			if(info->used > info->max)
			{
				fflush(info->file);
				info->used = 0;
			}
		}
		else
		{
			fprintf(info->file, "[%2.2d/%2.2d/%2.2d %2.2d:%2.2d:%2.2d] %s\n", (info->ts.tm_mon + 1), info->ts.tm_mday, (info->ts.tm_year % 100), info->ts.tm_hour, info->ts.tm_min, info->ts.tm_sec, message);
			fflush(info->file);
		}
	}
}
void Logger::log(unsigned logenum, int level, const char *message, ...)
{
	char buf[2048];
	va_list varg;
	va_start(varg, message);
	vsnprintf(buf, 2047, message, varg);
	buf[2047] = 0;
	va_end(varg);

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
		localtime_r(&t, &info->ts);
		info->last = t;
	}
	if(m_rollDate && info->ts.tm_mday != m_lastDateTime.tm_mday)
	{
#if defined(_REENTRANT)
		rLock.Lock();
#endif
		if(info->ts.tm_mday != m_lastDateTime.tm_mday)
		{
			memcpy(&m_lastDateTime, &info->ts, sizeof(tm));
			rollDate(t);
		}
#if defined(_REENTRANT)
		rLock.Unlock();
#endif
	}

	if(iter != m_logTable.end())
	{
		if(info->max > 0)
		{
			int tmp = fprintf(info->file, "[%2.2d/%2.2d/%2.2d %2.2d:%2.2d:%2.2d] %s\n", (info->ts.tm_mon + 1), info->ts.tm_mday, (info->ts.tm_year % 100), info->ts.tm_hour, info->ts.tm_min, info->ts.tm_sec, buf);
			info->used += tmp;
			if(info->used > info->max)
			{
				fflush(info->file);
				info->used = 0;
			}
		}
		else
		{
			fprintf(info->file, "[%2.2d/%2.2d/%2.2d %2.2d:%2.2d:%2.2d] %s\n", (info->ts.tm_mon + 1), info->ts.tm_mday, (info->ts.tm_year % 100), info->ts.tm_hour, info->ts.tm_min, info->ts.tm_sec, buf);
			fflush(info->file);
		}
	}
}

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
	localtime_r(&t, &now);

	snprintf(buf, 80, "%s%c%2.2d-%2.2d-%2.2d", m_dirPrefix.c_str(), file_sep, (now.tm_mon + 1), now.tm_mday, (now.tm_year % 100));
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

			mkdir(j, mode);
			close(handle);
			(*i) = file_sep;
		}
		else if(*(i + 1) == 0)
		{
			mkdir(j, mode);
		}
		i++;
	}
}
#ifdef EXTERNAL_DISTRO
};
#endif
};
