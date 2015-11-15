#ifndef __LOGGER_H__
#define __LOGGER_H__

// make the compiler shut up
#pragma warning (disable : 4786)

#include "Platform.h"
#include <string>
#include <map>
#include "Mutex.h"
#ifndef WIN32
#include <syslog.h>
#endif

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

/* Use these flags defined in syslog.h for the priority level
	LOG_EMERG,
	LOG_ALERT,
	LOG_CRIT,
	LOG_ERR,
	LOG_WARNING,
	LOG_NOTICE,
	LOG_INFO,
	LOG_DEBUG
*/

//-----------------------------------------------
// Windows doesn't have syslog (?) so we define the
// syslog warning levels here to satisfy the compiler
// but then ignore them in log() and logSimple()
//-----------------------------------------------

#ifdef WIN32
enum _WIN32_LOG_LEVELS_
{
	LOG_EMERG = 0,
	LOG_ALERT,
	LOG_CRIT,
	LOG_ERR,
	LOG_WARNING,
	LOG_NOTICE,
	LOG_INFO,
	LOG_DEBUG
};
#endif

enum ELogType
{
	eUseNone = 0,
	eUseLocalFile = 1,
	eUseSysLog = 2,
	eUseBoth = 3
};

class Logger
{
public:
	Logger(const char *programName, 
		   const ELogType logType = eUseLocalFile, 
		   const char *prefix = "logs", 
		   int level = 10, 
		   unsigned size = 0, 
		   bool rollDate = true);

	Logger(const char *prefix, 
			   int level, 
			   unsigned size, 
			   bool rollDate = true); // Backwards compatibility
	void LoggerInit(const char *programName);

	~Logger();
	void addLog(const char *id, unsigned logenum, int level, unsigned size);
	void addLog(const char *id, unsigned logenum);
	void removeLog(unsigned logenum);
	void updateLog(unsigned logenum, int level, unsigned size);
	void logSimple(unsigned logenum, int level, const char *message);
    void logSimpleWithSys(unsigned logenum, unsigned priority, int level, const char *message);
	void logWithSys(unsigned logenum, unsigned priority, int level, const char *message, ...);
	void log(unsigned logenum, int level, const char *message, ...);

	void flushAll();
	void flush(unsigned logenum);
	unsigned long getLogRolloverSize() { return m_logRolloverSize; }
	void setLogRolloverSize(unsigned long maxLogSize) { m_logRolloverSize = maxLogSize; }

private:
	void rollDate(time_t now);
	void cmkdir(const char *dir, int mode);
	void rollLog(LogInfo *logInfo);
	void checkLogRoll(LogInfo *logInfo, unsigned long lenToAdd);

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
	ELogType m_logType;
	unsigned long m_logRolloverSize;
};

extern const char file_sep;

};
#ifdef EXTERNAL_DISTRO
};
#endif

#endif
