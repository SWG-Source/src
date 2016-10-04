#ifndef _AUTOLOG_H
#define _AUTOLOG_H

#pragma warning (disable : 4786)
#include <stdio.h>
#include <time.h>

#ifdef EXTERNAL_DISTRO
namespace NAMESPACE
{
#endif
	namespace Base
	{
		class CAutoLog
		{
		public:
			enum eLogLevel {
				eLOG_NONE = 0,      // log entries are discarded
				eLOG_ERROR = 1,     // log errors only
				eLOG_ALERT = 2,     // log alerts and errors only
				eLOG_NORMAL = 3,   // log all normal events, no debug
				eLOG_DEBUG = 4      // log everything
			};

			// Global setting for all AutoLog instances for this application. Can be set on the fly. Default is eLOG_NORMAL.
			void SetLogLevel(eLogLevel loglevel);

			// Global setting for all AutoLog instances for this application. Can be set on the fly. Default is eLOG_ERROR.
			void SetPrintLevel(eLogLevel printlevel);

			// Global setting for all AutoLog instances for this application. Can be set on the fly. Default is eLOG_ERROR.
			void SetFlushLevel(eLogLevel flushlevel);

			// constructor, no file loaded
			CAutoLog();

			// version of constructor that calls Open(file)
			CAutoLog(const char * file);

			// destructor, will close file if opened
			~CAutoLog();

			// Open log file. If a file is already open, function will fail.
			// returns true if no error
			bool	Open(const char * file);

			// Close log file if opened.
			void	Close(void);

			// Make an entry in the log. Format and variable arguments are identical to printf.
			// severity will be compared to master log level to determine if the log entry will be made
			// severity will be compared to master print level to determine if the log entry will be printed to stdout
			void	Log(eLogLevel severity, char * format, ...);

			// Equivalent to the above with the approriate severity argument
			void	LogError(char * format, ...);
			void	LogAlert(char * format, ...);
			void	Log(char * format, ...);
			void	LogDebug(char * format, ...);

		private:
			FILE *  pFile;                  // current log file opened
			char *  pFilename;              // current log file opened
			int     nTodaysDayOfYear;       // remember the current day to detect change of day
			void	Archive(void);          // archives current log

			static eLogLevel nLogMask;            // master severity level
			static eLogLevel nPrintMask;          // master severity level
			static eLogLevel nFlushMask;          // master severity level
		};

		//-------------------------------------
		inline void CAutoLog::SetLogLevel(eLogLevel loglevel)
		{
			nLogMask = loglevel;
		}

		//-------------------------------------
		inline void CAutoLog::SetPrintLevel(eLogLevel printlevel)
		{
			nPrintMask = printlevel;
		}

		//-------------------------------------
		inline void CAutoLog::SetFlushLevel(eLogLevel flushlevel)
		{
			nFlushMask = flushlevel;
		}

		//-------------------------------------
		inline CAutoLog::CAutoLog()
		{
			pFilename = nullptr;
			pFile = (FILE *)-1;
			nTodaysDayOfYear = 0;
		}

		//-------------------------------------
		inline CAutoLog::CAutoLog(const char * file)
		{
			pFilename = nullptr;
			pFile = (FILE *)-1;
			Open(file);
		}

		//-------------------------------------
		inline CAutoLog::~CAutoLog()
		{
			Close();
		}
	};
#ifdef EXTERNAL_DISTRO
};
#endif

#endif
