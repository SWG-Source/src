#include "AutoLog.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

#ifdef EXTERNAL_DISTRO
namespace NAMESPACE
{
#endif
	namespace Base
	{
#ifdef WIN32
#include <direct.h>					// for NT directory commands
#define SLASHCHAR   "\\"
#define WRONGSLASH  '/'
#else
#define SLASHCHAR   "/"
#define WRONGSLASH  '\\'
#endif

		// set default values for global masks
		CAutoLog::eLogLevel CAutoLog::nLogMask = eLOG_NORMAL;     // what is logged in log files
		CAutoLog::eLogLevel CAutoLog::nPrintMask = eLOG_ERROR;    // what is printed on the screen
		CAutoLog::eLogLevel CAutoLog::nFlushMask = eLOG_ERROR;    // when is file flushed on every entry?

		// class info

		//-------------------------------------
		bool CAutoLog::Open(const char * filename)
			//-------------------------------------
		{
			if (nullptr == filename)
				return false;

			if (pFilename)
				return false;

			pFilename = strdup(filename);

			//  Sanitize slashes
			char *ptr;
			while ((ptr = strchr(pFilename, WRONGSLASH)) != nullptr)
				*ptr = SLASHCHAR[0];

			char    strPath[1024];
			strcpy(strPath, pFilename);
			ptr = strrchr(strPath, SLASHCHAR[0]);
			if (ptr != nullptr)
			{
				*ptr = 0;

#ifdef WIN32
				char    curdir[128];

				// remember current directory
				if (getcwd(curdir, sizeof(curdir)) == nullptr)
				{
					fprintf(stderr, "CAutoLog::Archive failed to get current directory!\n");
					free(pFilename);
					pFilename = nullptr;
					return false; // error, can't proceed
				}

				if (chdir(strPath))
				{
					// failed to find the directory, so we must create it
					if (mkdir(strPath))
					{
						fprintf(stderr, "CAutoLog::Archive failed to make directory: %s\n", strPath);
						free(pFilename);
						pFilename = nullptr;
						return false; // error, can't proceed
					}
				}
				else
				{
					// found the directory, but we don't want to be there, so go back
					chdir(curdir);
				}
#else
				struct stat SS;

				// see if directory exists
				if (stat(strPath, &SS))
				{
					// create directory
					if (mkdir(strPath, 0777))
					{
						fprintf(stderr, "CAutoLog::Archive failed to make directory: %s\n", strPath);
						free(pFilename);
						pFilename = nullptr;
						return false; // error, can't proceed
					}
				}
#endif
			}

			pFile = fopen(pFilename, "a+");
			if (pFile == nullptr || pFile == (FILE *)-1)
			{
				//fprintf(stderr,"CAutoLog::Open failed to open log file: %s\n",pFilename);
				Close();
				return false;
			}

			time_t t;
			time(&t);
			struct tm *ptm = localtime(&t);
			nTodaysDayOfYear = ptm->tm_yday;

			//printf("Open log file: %s\n",pFilename);
			return true;
		}

		//-------------------------------------
		void CAutoLog::Close(void)
			//-------------------------------------
		{
			if (pFile != (FILE *)-1 && pFile != nullptr)
			{
				fclose(pFile);
			}
			pFile = (FILE *)-1;

			free(pFilename);
			pFilename = nullptr;
		}

		//-------------------------------------
		void CAutoLog::LogError(char * format, ...)
		{
			char    strInput[1024];

			// parse format and input arguments
			va_list varg;
			va_start(varg, format);
			vsprintf(strInput, format, varg);
			va_end(varg);

			Log(eLOG_ERROR, strInput);
		}

		//-------------------------------------
		void CAutoLog::LogAlert(char * format, ...)
		{
			char    strInput[1024];

			// parse format and input arguments
			va_list varg;
			va_start(varg, format);
			vsprintf(strInput, format, varg);
			va_end(varg);

			Log(eLOG_ALERT, strInput);
		}

		//-------------------------------------
		void CAutoLog::Log(char * format, ...)
		{
			char    strInput[1024];

			// parse format and input arguments
			va_list varg;
			va_start(varg, format);
			vsprintf(strInput, format, varg);
			va_end(varg);

			Log(eLOG_NORMAL, strInput);
		}

		//-------------------------------------
		void CAutoLog::LogDebug(char * format, ...)
		{
			char    strInput[1024];

			// parse format and input arguments
			va_list varg;
			va_start(varg, format);
			vsprintf(strInput, format, varg);
			va_end(varg);

			Log(eLOG_DEBUG, strInput);
		}

		//-------------------------------------
		void CAutoLog::Log(eLogLevel severity, char *fmt, ...)
			//-------------------------------------
		{
			if (pFile == (FILE *)-1 || pFile == nullptr)
			{
				//fprintf(stderr,"CAutoLog::Log called with no file open!\n");
				return;
			}

			if (nLogMask == eLOG_NONE || nLogMask < severity)
			{
				if (nPrintMask == eLOG_NONE || nPrintMask < severity)
				{   // no point in parsing, we're doing nothing.
					return;
				}
			}

			char    strTime[128];
			char    strInput[1024];
			char    strOutput[1024 + 128];
			time_t  t;

			// parse format and input arguments
			va_list varg;
			va_start(varg, fmt);
			vsprintf(strInput, fmt, varg);
			va_end(varg);

			// make timestamp
			time(&t);
			strftime(strTime, sizeof(strTime), "[%m/%d/%Y %H:%M:%S]", localtime(&t));

			// construct output string
			snprintf(strOutput, 1152, "%s %s\n", strTime, strInput);

			// check to see if current file needs to be archived
			Archive();

			// log to file
			if (!(nLogMask == eLOG_NONE || nLogMask < severity))
			{
				fwrite(strOutput, strlen(strOutput), 1, pFile);
			}

			// print as standard output
			if (!(nPrintMask == eLOG_NONE || nPrintMask < severity))
			{
				printf("%s", strOutput);
			}

			// flush file buffer if error is high severity
			if (severity <= nFlushMask)
				fflush(pFile);
		}

		//-------------------------------------
		void CAutoLog::Archive(void)
			//-------------------------------------
		{
			if (pFile == (FILE *)-1 || pFile == nullptr)
			{
				//fprintf(stderr,"CAutoLog::Archive called with no file open!\n");
				return;
			}

			// check to see if it's time to archive
			time_t t;
			time(&t);
			struct tm *ptm = localtime(&t);
			if (ptm->tm_yday == nTodaysDayOfYear)
			{
				return; // nope, not time to archive
			}
			nTodaysDayOfYear = ptm->tm_yday;

			char    strPath[1024];
			char    strCurrent[1024];
			char    strTime[128];
			char *  pCurrent;

#ifdef WIN32
			char    curdir[128];
#else
			struct stat SS;
#endif

			strftime(strTime, sizeof(strTime), "%m%d%Y", localtime(&t));		// numerical time e.g. 041698

			strcpy(strCurrent, pFilename);
			pCurrent = strrchr(strCurrent, SLASHCHAR[0]);
			if (pCurrent != nullptr)
				*pCurrent = 0;
			else
				sprintf(strCurrent, ".");

			snprintf(strPath, 1024, "%s" SLASHCHAR "%s", strCurrent, strTime);                   // logs/041698

#ifdef WIN32
	// remember current directory
			if (getcwd(curdir, sizeof(curdir)) == nullptr)
			{
				fprintf(stderr, "CAutoLog::Archive failed to get current directory!\n");
				return; // error, can't proceed
			}

			if (chdir(strPath))
			{
				// failed to find the directory, so we must create it
				if (mkdir(strPath))
				{
					fprintf(stderr, "CAutoLog::Archive failed to make directory: %s\n", strPath);
					return; // error, can't proceed
				}
			}
			else
			{
				// found the directory, but we don't want to be there, so go back
				chdir(curdir);
			}
#else
	// see if directory exists
			if (stat(strPath, &SS))
			{
				// create directory
				if (mkdir(strPath, 0777))
				{
					fprintf(stderr, "CAutoLog::Archive failed to make directory: %s\n", strPath);
					return; // error, can't proceed
				}
			}
#endif

			pCurrent = strrchr(pFilename, SLASHCHAR[0]);
			if (pCurrent == nullptr)
				pCurrent = pFilename;
			else
				pCurrent++;

			sprintf(strCurrent, "%s" SLASHCHAR "%s", strPath, pCurrent);

			fflush(pFile);
			fclose(pFile);

			rename(pFilename, strCurrent);

			pFile = fopen(pFilename, "w+");
		}
	};
#ifdef EXTERNAL_DISTRO
};
#endif