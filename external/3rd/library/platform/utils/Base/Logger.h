#ifndef __LOGGER_H__
#define __LOGGER_H__

// make the compiler shut up
#pragma warning (disable : 4786)

#include "Platform.h"
#include <string>
#include <map>
#include "Mutex.h"

#ifdef EXTERNAL_DISTRO
namespace NAMESPACE 
{

#endif
namespace Base 
{

#define LOG_ALWAYS	-1
#define LOG_FILEONLY -2

struct LogInfo
{
	FILE *file;
	unsigned used;
	tm ts;
	time_t last;
	unsigned max;
	int level;
	std::string filename;
	std::string name;
};

class Logger
{
public:
	Logger(const char *prefix = "logs", int level = 10, unsigned size = 0, bool rollDate = true);
	~Logger();
	void addLog(const char *id, unsigned logenum, int level, unsigned size);
	void addLog(const char *id, unsigned logenum);
	void removeLog(unsigned logenum);
	void updateLog(unsigned logenum, int level, unsigned size);
	void logSimple(unsigned logenum, int level, const char *message);
	void log(unsigned logenum, int level, const char *message, ...);
	void flushAll();
	void flush(unsigned logenum);
private:
	void rollDate(time_t now);
	void cmkdir(const char *dir, int mode);
	unsigned m_defaultLevel;
	unsigned m_defaultSize;
	std::string m_lastDateText;
	tm m_lastDateTime;
	std::map<unsigned, LogInfo *> m_logTable;
	std::string m_logPrefix;
	std::string m_dirPrefix;
#if defined (_REENTRANT) || defined (_MT)
	Base::CMutex rLock;
#endif
	bool m_rollDate;
};

extern const char file_sep;

};
#ifdef EXTERNAL_DISTRO
};
#endif

#endif
