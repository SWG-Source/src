#include "sharedFoundation/FirstSharedFoundation.h"
#include "sharedFoundation/PlatformGlue.h"
#include <ctime>


int _stricmp(const char* string1, const char* string2)
{
    int first, second;

    do
    {
        first = toupper(*string1);
        second = toupper(*string2);
        string1++;
        string2++;
    } while (first && first==second);

    return (first - second);
}

char* _itoa(int value, char* stringOut, int radix)
{
  DEBUG_FATAL((radix != 10), ("itoa only supprts base 10"));
  sprintf(stringOut, "%d", value);
  return stringOut;
}

bool QueryPerformanceCounter(__int64* time)
{
  struct timeval tv;
  gettimeofday(&tv, nullptr);
  *time = static_cast<LARGE_INTEGER>(tv.tv_sec);
  *time = (*time * 1000000) + static_cast<LARGE_INTEGER>(tv.tv_usec);

  return TRUE;
}

bool QueryPerformanceFrequency(__int64 *freq)
{
    *freq = CLOCKS_PER_SEC;
    return TRUE;

}

void Sleep(DWORD msecs)
{
        //Sleep takes a time to sleep in milliseconds.
    usleep(msecs*1000);  //usleep works with microseconds
}

int GetLastError()
{
    return errno;
}

void OutputDebugString(const char* stringOut)
{
    fprintf(stderr,"%s",stringOut);
}


//File Support
BOOL WriteFile(FILE* hFile, const void* lpBuffer, DWORD numBytesToWrite, DWORD* numBytesWritten, void* unsup)
{
    int retval;

    if(unsup)
        return FALSE;  //Windows doesn't support this overrlap buffer

    retval = fwrite(lpBuffer, sizeof(char), numBytesToWrite, hFile);
    *numBytesWritten = retval;
    return (retval < 0) ? FALSE : TRUE;

}

BOOL ReadFile(FILE* hFile, void* lpBuffer, DWORD numBytesToRead, DWORD* numBytesRead, void* unsup)
{
  int retval;

  if(unsup)
    return FALSE;  //Windows doesn't support this overrlap buffer

  retval = fread(lpBuffer, sizeof(char), numBytesToRead, hFile);
  *numBytesRead = retval;
  return (retval <= 0) ? FALSE : TRUE;

}

DWORD SetFilePointer(FILE* hFile, long lDistanceToMove, long* lpDistanceToMoveHigh, DWORD dwMoveMethod)
{
  //Mimics the Win32 function to set the position of a file pointer.  return -1 on failure else current position.

  int retval;
  retval = fseek(hFile, lDistanceToMove, dwMoveMethod);

  return (retval == 0) ? ftell(hFile) : -1;
}

//TODO Consider using a call to open() instead of fopen() to get a better mapping of parameters to Win32
FILE* CreateFile(const char* fileName, DWORD access, DWORD shareMode, void* unsupA, DWORD creationDisposition, DWORD flagsAndAttributes, FILE* unsupB)
{
    FILE* retval = 0;

    DEBUG_FATAL(flagsAndAttributes != FILE_ATTRIBUTE_NORMAL, ("Unsupported File mode call to CreateFile()"));
    DEBUG_FATAL(unsupB != nullptr, ("Unsupported File mode call to CreateFile()"));
    //DEBUG_FATAL(shareMode != 0, ("Unsupported File mode call to CreateFile()"));
    DEBUG_FATAL(unsupA != 0, ("Unsupported File mode call to CreateFile()"));

    switch(creationDisposition)
    {
        case CREATE_NEW:
            retval = fopen(fileName, "r");
            if (retval)
            {
                fclose(retval);
                retval = nullptr;
            }
            else
            {
                fclose(retval);
                retval = fopen(fileName, "w");
            }
            break;

        case CREATE_ALWAYS:
            retval = fopen(fileName, "w");
            break;

        case OPEN_EXISTING:
            retval = fopen(fileName, "r");
            if ((access & GENERIC_WRITE) && retval)
            {
                fclose(retval);
                retval = fopen(fileName, "a");
                rewind(retval);
            }
            break;

        case OPEN_ALWAYS:
            if (access & GENERIC_WRITE)
            {
                retval = fopen(fileName, "a");
                rewind(retval);
            }
            else
            {
                retval = fopen(fileName, "r");
            }
            break;

        case TRUNCATE_EXISTING:
            DEBUG_FATAL(!(access & GENERIC_WRITE),("Must open truncate file with write access"));
            retval = fopen(fileName, "w");
            break;
    }

    return retval;
}


BOOL CloseHandle(FILE* hFile)
{
  int retval = fclose(hFile);
  return (retval == 0) ? TRUE : FALSE;
}

//TODO There must be a more elegant UNIX command to get file size...fstat() needs filename
DWORD GetFileSize(FILE* hFile, DWORD* lpHighSize)
{
    long int curPos;
    DWORD endPos;
    curPos = ftell(hFile);
    fseek(hFile, 0, SEEK_END);
    endPos = ftell(hFile);
    fseek(hFile, curPos, SEEK_SET);
    return endPos;
}

BOOL FileExists(const char* filename)
{
    BOOL retval = false;

	struct stat info;
	if (stat(filename, &info) == 0)
	{
		if (S_ISREG(info.st_mode))
			retval = true;
	}

    return retval;
}



//Memory Functions

void* VirtualAlloc(void* location, DWORD size, DWORD flAllocationType, DWORD flProtect)
{
	if (location)
		return location;

	return malloc(size);
}

BOOL VirtualFree(void* location, DWORD size, DWORD freeType)
{
  free(location);
  return TRUE;
}

BOOL VirtualProtect(void* location, DWORD size, DWORD newProtect, DWORD* oldProect)
{
        //Unused.  Function stub for compatibility
    return TRUE;
}

BOOL IsBadReadPtr(const void* location, unsigned int size)
{
        //unused
    return TRUE;
}

void MessageBox (void* unused, const char* message, const char* unused2, int flags)
{
    OutputDebugString(message);
}

char *ConvertCommandLine(int argc, char ** argv)
{
	static char buffer[2048];

	int totalSize=0;
	for (int i=0; i<argc; ++i)
	{
		totalSize+=strlen(argv[i])+1;
	}

	if(totalSize>1023)
	{
		int argIndex = 0;
		fprintf(stderr, "Exceeded command line args length of %d. Total size is %d\n", sizeof(buffer), totalSize);
		for(argIndex = 0; argIndex < argc; ++argIndex)
		{
			fprintf(stderr, "argv[%d]=\"%s\"\n", argIndex, argv[argIndex]);
		}
		FATAL(true,("Command line exceeds maximum length."));
	}
	
	buffer[0]='\0';

	for (int i=0; i<argc; ++i)
	{
		strcat(buffer,argv[i]);
		if (i!=argc-1)
			{
				strcat(buffer," ");
			}
	}

	return buffer;
}
