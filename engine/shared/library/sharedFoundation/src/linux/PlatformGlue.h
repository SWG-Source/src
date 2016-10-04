#ifndef INCLUDED_MISC_H
#define INCLUDED_MISC_H

#include <netinet/in.h>
#include <sys/time.h>
#include <errno.h>

#include <fcntl.h>
#include <unistd.h>
#include <pthread.h>

typedef unsigned short int WORD;
typedef unsigned long int DWORD;
typedef bool BOOL;
typedef long long     __int64; //lint !e13 !e19 // Error: 13 (Bad type), Error: 19 (Useless declaration) // -TRF- Lint preprocessor discrepency, @todo look into this.
typedef __int64         LARGE_INTEGER;

const BOOL FALSE = 0;
const BOOL TRUE = 1;


int _stricmp(const char* string1, const char* string2);

//String to numeric conversions
char* _itoa(int value, char* stringOut, int radix);

//Format specifier for non-portable printf
#define UINT64_FORMAT_SPECIFIER "%llu"
#define INT64_FORMAT_SPECIFIER "%lli"

//Constant definition macro for 64 bit values
#define UINT64_LITERAL(a) a ## ull
#define INT64_LITERAL(a) a ## ll

bool QueryPerformanceCounter(__int64 *time);
bool QueryPerformanceFrequency(__int64 *freq);
void Sleep(DWORD msecs);
int GetLastError();
void OutputDebugString(const char* stringOut);

//File Support
#include <stdio.h>
#include <sys/stat.h>
#include <unistd.h>

typedef FILE* HANDLE; 

#define INVALID_HANDLE_VALUE nullptr //Have to use a #define because this may be a FILE*


const int GENERIC_READ = 1 << 0;
const int GENERIC_WRITE = 1 << 1;

const int CREATE_NEW = 1;
const int CREATE_ALWAYS = 2;
const int OPEN_EXISTING = 3;
const int OPEN_ALWAYS = 4;
const int TRUNCATE_EXISTING = 5;

const DWORD FILE_ATTRIBUTE_NORMAL = 1;

const int FILE_CURRENT = SEEK_CUR;
const int FILE_BEGIN = SEEK_SET;
const int FILE_END = SEEK_END;

const int FILE_SHARE_READ = 1;

BOOL WriteFile(FILE* hFile, const void* lpBuffer, DWORD numBytesToWrite, DWORD* numBytesWritten, void* unsup=nullptr);
BOOL ReadFile(FILE* hFile, void* lpBuffer, DWORD numBytesToWrite, DWORD* numBytesRead, void* unsup=nullptr);
DWORD SetFilePointer(FILE* hFile, long lDistanceToMove, long* lpDistanceToMoveHigh, DWORD dwMoveMethod);
FILE* CreateFile(const char* fileName, DWORD access, DWORD shareMode, void* unsupA, DWORD creationDisposition, DWORD flagsAndAttributes, FILE* unsup2);
BOOL CloseHandle(FILE* hFile);
DWORD GetFileSize(FILE* hFile, DWORD* lpHighSize);
BOOL FileExists(const char* fileName);


//Memory Support

const int MEM_RESERVE = 0;
const int MEM_COMMIT = 1;
const int PAGE_READWRITE = 0;
const int MEM_RELEASE = 0;
const int PAGE_NOACCESS = 0;

void* VirtualAlloc(void* location, DWORD size, DWORD flAllocationType, DWORD flProtect);
BOOL VirtualFree(void* location, DWORD size, DWORD freeType);
BOOL VirtualProtect(void* location, DWORD size, DWORD newProtect, DWORD* oldProect);
BOOL IsBadReadPtr(const void* location, unsigned int size);

//Misc output stuff

const int MB_OK = 0;
const int MB_ICONEXCLAMATION = 1;
void MessageBox (void* unused, const char* message, const char* unused2, int flags);
char *ConvertCommandLine(int argc, char ** argv);

//String functions
#define _vsnprintf vsnprintf

//-----------------------------------------------------------------

inline int _strnicmp (const char * a, const char * b, size_t count)
{
  return ::strncasecmp (a, b, count);
}

//-----------------------------------------------------------------

inline int _open(const char *filename, int oflag)                   { return open(filename, oflag); }
inline int _open(const char *filename, int oflag, mode_t pmode)     { return open(filename, oflag, pmode); }
inline int _creat(const char *filename, mode_t pmode)               { return creat(filename, pmode); }
inline int _close(int fd)                                           { return close(fd); }
inline int _write(int fd, const void *buffer, unsigned int count)   { return write(fd, buffer, count); }
inline int _dup(int fd)                                             { return dup(fd); }
inline int _dup2(int oldfd, int newfd)                              { return dup2(oldfd, newfd); }
inline long _lseek(int fd, long offset, int origin)                 { return lseek(fd, offset, origin); }
inline int _read(int fd, void *buffer, unsigned int count)          { return read(fd, buffer, count); }
inline long _tell(int fd)                                           { return _lseek(fd,0,SEEK_CUR); }
inline int _umask(int pmode)                                        { return umask(pmode); }
inline int _unlink(const char *pathname)                            { return unlink(pathname); }

//-----------------------------------------------------------------

#endif //INCLUDED_MISC_H
