// ======================================================================
//
// FileStreamerThread.h
// Portions copyright 1999 Bootprint Entertainment
// Portions copyright 2002 Sony Online Entertainment
// All Rights Reserved
//
// ======================================================================

#ifndef INCLUDED_FileStreamerThread_H
#define INCLUDED_FileStreamerThread_H

// ======================================================================

#include "fileInterface/AbstractFile.h"
#include "sharedFile/FileStreamer.h"

class FileStreamerFile;
class Gate;
class MemoryBlockManager;
class Mutex;
class OsFile;
class Semaphore;
class MemoryBlockManager;
class Thread;
template <class T> class TypedThreadHandle;
typedef TypedThreadHandle<Thread> ThreadHandle;

// ======================================================================

// Encapsulates thread for file access
//
// This class represents the file streaming thread.  It should only be accessed
// by the FileStreamer class.
//
// All requests, with the exceptions of reads and quits, and responded to immediately.
// The "submit-request" paradigm is used to consistancy.  Read and quit requests are 
// put in multiple queues and serviced by priority (audio-visual before plain data before low, etc.)
//
// AV request will "interrupt" any current data request, and all reads are only serviced
// 16K at a time.

class FileStreamerThread
{
	friend class FileStreamer::File;

public:

	class  Request;

public:

	static void    install();

private:

	static bool                ms_installed;
	static ThreadHandle        ms_threadHandle;
	static Semaphore           ms_eventsPending;
	static Mutex               ms_queueCriticalSection;
	static volatile Request   *ms_firstAudioVideoRequest;
	static volatile Request   *ms_lastAudioVideoRequest;
	static volatile Request   *ms_firstDataRequest;
	static volatile Request   *ms_lastDataRequest;
	static volatile Request   *ms_firstLowRequest;
	static volatile Request   *ms_lastLowRequest;

private:

	static void remove(void);

	static void verifyOpen(const char *function, int handle);
	static void threadRoutine();
	static void submitRequest(Request *request);

	static void processRead(volatile Request *request);
	static void processQuit(volatile Request *request);
};

// ======================================================================

class FileStreamerThread::Request
{
	friend class FileStreamerThread;
	friend class FileStreamerFile;

public:

	static void           install();
	static void *operator new(size_t size);
	static void  operator delete(void *pointer);

public:

	//initialized to unknown.  Must be changed before usage or it will debug_fatal
	enum Type
	{
		Unknown,
		Quit,
		Read
	};

	volatile Request           *next;
	Type                        type;
	OsFile                     *osFile;
	void                       *buffer;
	int                         offset;
	int                         bytesToBeRead;
	int                         bytesRead;

	// event to trigger when finished servicing request
	Gate                       *gate;

	// used to place in correct queue for priority service
	AbstractFile::PriorityType  priority;

	// storage held by game thread used to pass back return value
	int                        *returnValue;

public:

	Request();
	~Request();

private:

	static MemoryBlockManager  *ms_memoryBlockManager;

private:

	static void remove();
};

// ======================================================================

#endif
